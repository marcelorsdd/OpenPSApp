#include "http/server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <vector>

#include "util/log.hpp"
#include "util/stats.hpp"
#include "util/thread.hpp"

namespace playtrack::http {
namespace {
constexpr int kMaxActive = 32;
constexpr size_t kMaxHeaderBytes = 16 * 1024;
constexpr size_t kMaxBodyBytes = 64 * 1024;

size_t ContentLength(std::string_view headers) {
    size_t pos = 0;
    while ((pos = headers.find("\r\n", pos)) != std::string_view::npos) {
        pos += 2;
        std::string_view line = headers.substr(pos, headers.find("\r\n", pos) - pos);
        if (line.size() < 15 || strncasecmp(line.data(), "content-length:", 15) != 0) continue;
        size_t value = 0;
        for (char c : line.substr(15)) {
            if (c >= '0' && c <= '9') value = value * 10 + static_cast<size_t>(c - '0');
            else if (c != ' ' && c != '\t') return 0;
            if (value > (1u << 30)) return value;
        }
        return value;
    }
    return 0;
}

const char *StatusText(int status) {
    switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 500: return "Internal Server Error";
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

void SetTimeouts(int fd) {
    timeval rcv {5, 0};
    timeval snd {30, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv, sizeof(rcv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &snd, sizeof(snd));
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
}

void Send(int fd, const Response &res, bool head_only) {
    int status = res.status;
    int file_fd = -1;
    int64_t length = static_cast<int64_t>(res.body.size());
    if (!res.file_path.empty()) {
        struct stat st {};
        file_fd = open(res.file_path.c_str(), O_RDONLY);
        int64_t offset = static_cast<int64_t>(res.file_offset);
        bool ok = file_fd >= 0 && fstat(file_fd, &st) == 0 && offset <= st.st_size;
        if (ok) {
            length = res.file_length < 0 ? st.st_size - offset : res.file_length;
            ok = offset + length <= st.st_size && lseek(file_fd, offset, SEEK_SET) == offset;
        }
        if (!ok) {
            if (file_fd >= 0) close(file_fd);
            Send(fd, Error(404, "file not found"), head_only);
            return;
        }
    }

    std::string head = "HTTP/1.1 " + std::to_string(status) + " " + StatusText(status) + "\r\n" +
                       "Content-Type: " + res.content_type + "\r\n" +
                       "Content-Length: " + std::to_string(length) + "\r\n" +
                       "Cache-Control: " + res.cache_control + "\r\n" +
                       (res.content_encoding.empty() ? "" : "Content-Encoding: " + res.content_encoding + "\r\n") +
                       "Access-Control-Allow-Origin: *\r\n" +
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
}

Server::~Server() {
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

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 || listen(listen_fd_, 16) != 0) {
        if (err) *err = "port " + std::to_string(port) + ": " + std::strerror(errno);
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }
    return true;
}

void Server::Serve() {
    for (;;) {
        int fd = accept(listen_fd_, nullptr, nullptr);
        if (fd < 0) {
            if (errno == EINTR || errno == ECONNABORTED) continue;
            log::Error("accept: %s", std::strerror(errno));
            usleep(100 * 1000);
            continue;
        }
        SetTimeouts(fd);
        if (active_.load() >= kMaxActive) {
            Send(fd, Error(503, "busy"), false);
            close(fd);
            continue;
        }
        active_++;
        bool spawned = Spawn([this, fd]() {
            Handle(fd);
            close(fd);
            active_--;
        });
        if (!spawned) {
            active_--;
            Send(fd, Error(503, "could not start worker"), false);
            close(fd);
        }
    }
}

void Server::Handle(int fd) {
    std::string raw;
    char buf[4096];
    while (raw.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return;
        raw.append(buf, static_cast<size_t>(n));
        if (raw.size() > kMaxHeaderBytes) {
            Send(fd, Error(400, "request too large"), false);
            return;
        }
    }

    size_t line_end = raw.find("\r\n");
    std::string line = raw.substr(0, line_end);
    size_t sp1 = line.find(' ');
    size_t sp2 = line.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) {
        Send(fd, Error(400, "bad request line"), false);
        return;
    }

    Request req;
    req.method = line.substr(0, sp1);
    ParseTarget(std::string_view(line).substr(sp1 + 1, sp2 - sp1 - 1), &req.path, &req.query);

    size_t header_end = raw.find("\r\n\r\n") + 4;
    size_t length = ContentLength(std::string_view(raw).substr(0, header_end));
    if (length > kMaxBodyBytes) {
        Send(fd, Error(400, "body too large"), false);
        return;
    }
    req.body = raw.substr(header_end);
    while (req.body.size() < length) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n < 0 && errno == EINTR) continue;
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
        res = Error(500, e.what());
    }
    if (res.status >= 400) log::Info("%s %s -> %d", req.method.c_str(), req.path.c_str(), res.status);
    Send(fd, res, req.method == "HEAD");
}

}
