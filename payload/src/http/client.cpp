#include "http/client.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>

namespace playtrack::http {
bool LocalGet(uint16_t port, const std::string &path, std::string *body, int timeout_ms) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;
    timeval tv {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    std::string request = "GET " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    bool ok = connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0 &&
              send(fd, request.data(), request.size(), 0) == static_cast<ssize_t>(request.size());

    std::string raw;
    char buf[2048];
    while (ok) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) break;
        raw.append(buf, static_cast<size_t>(n));
        if (raw.size() > 256 * 1024) break;
    }
    close(fd);

    size_t header_end = raw.find("\r\n\r\n");
    if (!ok || header_end == std::string::npos || raw.compare(0, 12, "HTTP/1.1 200") != 0) return false;
    *body = raw.substr(header_end + 4);
    return true;
}

}
