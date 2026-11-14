#include "http/server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <exception>
#include <map>
#include <string>
#include <vector>

#include "util/log.hpp"
#include "util/stats.hpp"
#include "util/thread.hpp"

namespace openpsapp::http {
namespace {
using Clock = std::chrono::steady_clock;

constexpr size_t kMaxHeaderBytes = 16 * 1024;
constexpr size_t kMaxBodyBytes = 64 * 1024;
constexpr int64_t kSendTimeoutMs = 30 * 1000;
constexpr int64_t kRejectTimeoutMs = 200;

size_t ContentLength(const std::map<std::string, std::string> &headers) {
    auto it = headers.find("content-length");
    if (it == headers.end()) return 0;
    size_t value = 0;
    for (char c : it->second) {
        if (c >= '0' && c <= '9') value = value * 10 + static_cast<size_t>(c - '0');
        else if (c != ' ' && c != '\t') return 0;
        if (value > (1u << 30)) return value;
    }
    return value;
}

const char *StatusText(int status) {
    switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 409: return "Conflict";
    case 413: return "Content Too Large";
    case 431: return "Request Header Fields Too Large";
    case 500: return "Internal Server Error";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    default: return "Unknown";
    }
}

bool SendAll(int fd, const char *data, size_t size) {
    while (size > 0) {
        ssize_t n = send(fd, data, size, 0);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return false;
        data += n;
        size -= static_cast<size_t>(n);
    }
    return true;
}

timeval Millis(int64_t ms) {
    timeval tv{};
    tv.tv_sec = static_cast<time_t>(ms / 1000);
    tv.tv_usec = static_cast<suseconds_t>((ms % 1000) * 1000);
    return tv;
}

void SetSendTimeout(int fd, int64_t ms) {
    timeval tv = Millis(ms);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

void DisableSigpipe([[maybe_unused]] int fd) {
#ifdef SO_NOSIGPIPE
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
}

ssize_t RecvBefore(int fd, char *buf, size_t size, Clock::time_point deadline) {
    for (;;) {
        int64_t left = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()).count();
        if (left <= 0) return -1;
        timeval tv = Millis(left);
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ssize_t n = recv(fd, buf, size, 0);
        if (n < 0 && errno == EINTR) continue;
        return n;
    }
}

void Send(int fd, const Response &res, bool head_only) {
    int status = res.status;
    int file_fd = -1;
    int64_t length = static_cast<int64_t>(res.body.size());
    if (!res.file_path.empty()) {
        struct stat st{};
        file_fd = open(res.file_path.c_str(), O_RDONLY);
        int64_t offset = static_cast<int64_t>(res.file_offset);
        bool ok = file_fd >= 0 && fstat(file_fd, &st) == 0 && offset <= st.st_size;
        if (ok) {
            length = res.file_length < 0 ? st.st_size - offset : res.file_length;
            ok = offset + length <= st.st_size && lseek(file_fd, offset, SEEK_SET) == offset;
        }
        if (!ok) {
            if (file_fd >= 0) close(file_fd);
            Send(fd, Error(404, "not_found"), head_only);
            return;
        }
    }

    std::string head = "HTTP/1.1 " + std::to_string(status) + " " + StatusText(status) + "\r\n" +
                       "Content-Type: " + res.content_type + "\r\n" + "Content-Length: " + std::to_string(length) +
                       "\r\n" + "Cache-Control: " + res.cache_control + "\r\n" +
                       (res.content_encoding.empty() ? "" : "Content-Encoding: " + res.content_encoding + "\r\n") +
                       "Connection: close\r\n\r\n";
    bool ok = SendAll(fd, head.data(), head.size());

    if (ok && !head_only) {
        if (file_fd >= 0) {
            std::vector<char> buf(64 * 1024);
            int64_t left = length;
            while (ok && left > 0) {
                ssize_t n = read(file_fd, buf.data(), static_cast<size_t>(std::min<int64_t>(left, buf.size())));
                if (n <= 0) break;
                ok = SendAll(fd, buf.data(), static_cast<size_t>(n));
                left -= n;
            }
        } else {
            SendAll(fd, res.body.data(), res.body.size());
        }
    }
    if (file_fd >= 0) close(file_fd);
}

void Reject(int fd, std::string_view why) {
    SetSendTimeout(fd, kRejectTimeoutMs);
    Send(fd, Error(503, why), false);
    close(fd);
}

void WakeAccept(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    close(fd);
}
}

Server::~Server() {
    Stop();
    if (listen_fd_ >= 0) close(listen_fd_);
}

bool Server::Listen(uint16_t port, std::string *err) {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        if (err) *err = std::string("socket: ") + std::strerror(errno);
        return false;
    }
    int one = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 || listen(listen_fd_, 16) != 0) {
        if (err) *err = "port " + std::to_string(port) + ": " + std::strerror(errno);
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    socklen_t addr_len = sizeof(addr);
    getsockname(listen_fd_, reinterpret_cast<sockaddr *>(&addr), &addr_len);
    port_ = ntohs(addr.sin_port);
    return true;
}

void Server::Serve() {
    serving_ = true;
    while (!stop_) {
        sockaddr_in addr{};
        socklen_t addr_len = sizeof(addr);
        int fd = accept(listen_fd_, reinterpret_cast<sockaddr *>(&addr), &addr_len);
        if (fd < 0) {
            if (errno == EINTR || errno == ECONNABORTED) continue;
            log::Error("accept: %s", std::strerror(errno));
            usleep(100 * 1000);
            continue;
        }
        if (stop_) {
            close(fd);
            break;
        }
        DisableSigpipe(fd);
        if (active_.load() >= limits_.max_active) {
            Reject(fd, "busy");
            continue;
        }
        char peer[INET_ADDRSTRLEN] = "";
        inet_ntop(AF_INET, &addr.sin_addr, peer, sizeof(peer));
        active_++;
        bool spawned = Spawn([this, fd, peer = std::string(peer)]() {
            Handle(fd, peer);
            close(fd);
            active_--;
        });
        if (!spawned) {
            active_--;
            Reject(fd, "busy");
        }
    }
    serving_ = false;
}

void Server::Stop() {
    stop_ = true;
    while (serving_) {
        WakeAccept(port_);
        usleep(10 * 1000);
    }
    while (active_ > 0) usleep(10 * 1000);
}

void Server::Handle(int fd, const std::string &peer) {
    SetSendTimeout(fd, kSendTimeoutMs);
    Clock::time_point deadline = Clock::now() + std::chrono::milliseconds(limits_.request_timeout_ms);
    std::string raw;
    char buf[4096];
    while (raw.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = RecvBefore(fd, buf, sizeof(buf), deadline);
        if (n <= 0) return;
        raw.append(buf, static_cast<size_t>(n));
        if (raw.size() > kMaxHeaderBytes) {
            Send(fd, Error(431, "headers_too_large"), false);
            return;
        }
    }

    size_t line_end = raw.find("\r\n");
    std::string line = raw.substr(0, line_end);
    size_t sp1 = line.find(' ');
    size_t sp2 = line.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) {
        Send(fd, Error(400, "bad_request"), false);
        return;
    }

    Request req;
    req.method = line.substr(0, sp1);
    req.peer = peer;
    ParseTarget(std::string_view(line).substr(sp1 + 1, sp2 - sp1 - 1), &req.path, &req.query);

    size_t header_end = raw.find("\r\n\r\n") + 4;
    ParseHeaders(std::string_view(raw).substr(0, header_end), &req.headers);
    if (!IsAllowedHost(Header(req, "host"))) {
        Send(fd, Error(403, "bad_host"), req.method == "HEAD");
        return;
    }
    size_t length = ContentLength(req.headers);
    if (length > kMaxBodyBytes) {
        Send(fd, Error(413, "too_large"), false);
        return;
    }
    req.body = raw.substr(header_end);
    while (req.body.size() < length) {
        ssize_t n = RecvBefore(fd, buf, sizeof(buf), deadline);
        if (n <= 0) return;
        req.body.append(buf, static_cast<size_t>(n));
    }
    req.body.resize(std::min(req.body.size(), length));

    stats::Get().requests++;
    Response res;
    try {
        res = router_.Dispatch(req);
    } catch (const std::exception &e) {
        log::Error("%s %s threw: %s", req.method.c_str(), req.path.c_str(), e.what());
        res = Error(500, "internal");
    }
    if (res.status >= 400)
        log::Info("%s %s from %s -> %d", req.method.c_str(), req.path.c_str(), peer.c_str(), res.status);
    Send(fd, res, req.method == "HEAD");
}

}
