#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>
#include <thread>

namespace tls_fixture {
extern const std::string_view kCaA;
extern const std::string_view kServerACert;
extern const std::string_view kServerAKey;
extern const std::string_view kCaB;
extern const std::string_view kServerBCert;
extern const std::string_view kServerBKey;

class TestTlsServer {
public:
    enum class Mode { Echo, Chunked, Silent };

    TestTlsServer(std::string_view cert_pem, std::string_view key_pem, Mode mode);
    ~TestTlsServer();
    TestTlsServer(const TestTlsServer &) = delete;
    TestTlsServer &operator=(const TestTlsServer &) = delete;

    uint16_t port() const { return port_; }

private:
    void Run();
    void Serve(int fd);

    std::string cert_;
    std::string key_;
    Mode mode_;
    int listener_ = -1;
    uint16_t port_ = 0;
    std::atomic<bool> stop_{false};
    std::thread thread_;
};

}
