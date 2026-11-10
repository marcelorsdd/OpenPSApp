#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include "http/http.hpp"

namespace openpsapp::http {
class Server {
public:
    explicit Server(const Router &router) : router_(router) {}
    ~Server();

    bool Listen(uint16_t port, std::string *err);
    void Serve();

private:
    void Handle(int fd);

    const Router &router_;
    int listen_fd_ = -1;
    std::atomic<int> active_{0};
};

}
