#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include "http/http.hpp"

namespace openpsapp::http {
struct ServerLimits {
    int max_active = 32;
    int request_timeout_ms = 10000;
};

class Server {
public:
    explicit Server(const Router &router, ServerLimits limits = {}) : router_(router), limits_(limits) {}
    ~Server();

    bool Listen(uint16_t port, std::string *err);
    uint16_t port() const { return port_; }
    void Serve();
    void Stop();

private:
    void Handle(int fd, const std::string &peer);

    const Router &router_;
    ServerLimits limits_;
    int listen_fd_ = -1;
    uint16_t port_ = 0;
    std::atomic<int> active_{0};
    std::atomic<bool> stop_{false};
    std::atomic<bool> serving_{false};
};

}
