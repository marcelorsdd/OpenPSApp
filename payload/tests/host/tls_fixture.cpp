#include "tls_fixture.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

namespace tls_fixture {
const std::string_view kCaA = R"PEM(-----BEGIN CERTIFICATE-----
MIIBgDCCASagAwIBAgIUZnrm0gqfTQdF/Ctw1+kPOA63N7YwCgYIKoZIzj0EAwIw
HjEcMBoGA1UEAwwTT3BlblBTQXBwIFRlc3QgQ0EgQTAeFw0yNjExMTQxNTU4MzJa
Fw00NjExMDkxNTU4MzJaMB4xHDAaBgNVBAMME09wZW5QU0FwcCBUZXN0IENBIEEw
WTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATu96tgUZ0iHDzD5/n4I37oJHj+nz8r
xr9RWKmQGtly2P0KEoN2K2IQnCuTCUW+v5ttVLJfc27nuy86yr8qsmaKo0IwQDAd
BgNVHQ4EFgQU2/xBKIB9cKVTB6cwod7EnLVKs9AwDwYDVR0TAQH/BAUwAwEB/zAO
BgNVHQ8BAf8EBAMCAQYwCgYIKoZIzj0EAwIDSAAwRQIgJVg87RKcfoztd79lWO1v
v5AEwYzXtHsseBBOAB3KfkMCIQDeAUEmFspYeDoHEfUZ8/pXemXNrRmRiflAWA4U
3jLZyA==
-----END CERTIFICATE-----
)PEM";

const std::string_view kServerACert = R"PEM(-----BEGIN CERTIFICATE-----
MIIBeTCCAR+gAwIBAgIUdUL8oUF4ccGbVZUGMOqSRCXTLakwCgYIKoZIzj0EAwIw
HjEcMBoGA1UEAwwTT3BlblBTQXBwIFRlc3QgQ0EgQTAeFw0yNjExMTQxNTU4MzJa
Fw00NjExMDkxNTU4MzJaMBQxEjAQBgNVBAMMCWxvY2FsaG9zdDBZMBMGByqGSM49
AgEGCCqGSM49AwEHA0IABMcv+snzU25omuwitefos/y27D0sawpt8s4mNs881FRs
3umKSsqparRGVE/1vBvEzesg6iLJouicgvhclW2R1PejRTBDMBQGA1UdEQQNMAuC
CWxvY2FsaG9zdDAJBgNVHRMEAjAAMAsGA1UdDwQEAwIHgDATBgNVHSUEDDAKBggr
BgEFBQcDATAKBggqhkjOPQQDAgNIADBFAiEAgknKnTBqSTMJ0GcRl5Uz7QsrZSud
FLyvFvq1E/Kb3Q8CIAWYM8AoDvigFJsg/21NbwrLSlwxDM+S2KSpx73lOW5E
-----END CERTIFICATE-----
)PEM";

const std::string_view kServerAKey = R"PEM(-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIP/kO1lHjBuE1lbqP83qX0NWibMED1QWYHaDMnekCzUDoAoGCCqGSM49
AwEHoUQDQgAExy/6yfNTbmia7CK15+iz/LbsPSxrCm3yziY2zzzUVGze6YpKyqlq
tEZUT/W8G8TN6yDqIsmi6JyC+FyVbZHU9w==
-----END EC PRIVATE KEY-----
)PEM";

const std::string_view kCaB = R"PEM(-----BEGIN CERTIFICATE-----
MIIBgTCCASagAwIBAgIUEgMFTYd3yVX1PA2ElwnC3Fg24p0wCgYIKoZIzj0EAwIw
HjEcMBoGA1UEAwwTT3BlblBTQXBwIFRlc3QgQ0EgQjAeFw0yNjExMTQxNTU4MzJa
Fw00NjExMDkxNTU4MzJaMB4xHDAaBgNVBAMME09wZW5QU0FwcCBUZXN0IENBIEIw
WTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAToEW2kI00ZOA0ne3Qvq+qqUZTmQLjT
i2umlUSNiIPXUjXyT0i5Fy6QTLRmA7rUkIODbZH2IEly9IT9INqhDJeLo0IwQDAd
BgNVHQ4EFgQUxQMgZ1hL0JoaDYCykvw3ESDBTvQwDwYDVR0TAQH/BAUwAwEB/zAO
BgNVHQ8BAf8EBAMCAQYwCgYIKoZIzj0EAwIDSQAwRgIhALYIbV8N5IssEk/Bub9i
C6dOMrzU1jSXgU7EM/F1SpWkAiEA/b+6RVBGJwelHJMQVeAYKl4IHRfN7kvm+uYV
hlh6Kx4=
-----END CERTIFICATE-----
)PEM";

const std::string_view kServerBCert = R"PEM(-----BEGIN CERTIFICATE-----
MIIBeDCCAR+gAwIBAgIUC97ruwyluKdNWjRkoLT2WDpC1ZEwCgYIKoZIzj0EAwIw
HjEcMBoGA1UEAwwTT3BlblBTQXBwIFRlc3QgQ0EgQjAeFw0yNjExMTQxNTU4MzJa
Fw00NjExMDkxNTU4MzJaMBQxEjAQBgNVBAMMCWxvY2FsaG9zdDBZMBMGByqGSM49
AgEGCCqGSM49AwEHA0IABCZFAcJYYd+NheD6iDFm0M/n0fCTkNZCtp6BjPTpstWF
bf2MIIEpNNPLEOCYZG3d3ycHgz1Nvi0Gx5vUf4rYGyKjRTBDMBQGA1UdEQQNMAuC
CWxvY2FsaG9zdDAJBgNVHRMEAjAAMAsGA1UdDwQEAwIHgDATBgNVHSUEDDAKBggr
BgEFBQcDATAKBggqhkjOPQQDAgNHADBEAiA8UU3UtvnO9ZckV5fCAgdcFn7l3erZ
XjsAslFFQpQMaAIgLL7DYViYuIj4oM3BhrpTWP4ouq9SmIiA/sv6nHTZvjg=
-----END CERTIFICATE-----
)PEM";

const std::string_view kServerBKey = R"PEM(-----BEGIN EC PRIVATE KEY-----
MHcCAQEEICqApuEq9Fn/RY3n0oviHLwlfDlU4yUe52Sh91Wg3oVMoAoGCCqGSM49
AwEHoUQDQgAEJkUBwlhh342F4PqIMWbQz+fR8JOQ1kK2noGM9Omy1YVt/YwggSk0
08sQ4Jhkbd3fJweDPU2+LQbHm9R/itgbIg==
-----END EC PRIVATE KEY-----
)PEM";

namespace {
int EntropySource(void *, unsigned char *out, size_t len, size_t *olen) {
    arc4random_buf(out, len);
    *olen = len;
    return 0;
}

int SendCb(void *ctx, const unsigned char *buf, size_t len) {
    ssize_t n = send(*static_cast<int *>(ctx), buf, len, 0);
    return n < 0 ? MBEDTLS_ERR_SSL_INTERNAL_ERROR : int(n);
}

int RecvCb(void *ctx, unsigned char *buf, size_t len) {
    ssize_t n = recv(*static_cast<int *>(ctx), buf, len, 0);
    return n < 0 ? MBEDTLS_ERR_SSL_INTERNAL_ERROR : int(n);
}

struct ServerTls {
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;

    ServerTls() {
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&cert);
        mbedtls_pk_init(&key);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&drbg);
    }
    ~ServerTls() {
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_config_free(&conf);
        mbedtls_x509_crt_free(&cert);
        mbedtls_pk_free(&key);
        mbedtls_ctr_drbg_free(&drbg);
        mbedtls_entropy_free(&entropy);
    }
};

std::string Field(const std::string &request, const char *name) {
    size_t at = request.find(name);
    if (at == std::string::npos) return "";
    at += std::strlen(name);
    return request.substr(at, request.find("\r\n", at) - at);
}
}

TestTlsServer::TestTlsServer(std::string_view cert_pem, std::string_view key_pem, Mode mode)
    : cert_(cert_pem), key_(key_pem), mode_(mode) {
    listener_ = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    bind(listener_, reinterpret_cast<sockaddr *>(&addr), sizeof addr);
    listen(listener_, 4);
    socklen_t len = sizeof addr;
    getsockname(listener_, reinterpret_cast<sockaddr *>(&addr), &len);
    port_ = ntohs(addr.sin_port);
    thread_ = std::thread([this] { Run(); });
}

TestTlsServer::~TestTlsServer() {
    stop_ = true;
    thread_.join();
    close(listener_);
}

void TestTlsServer::Run() {
    while (!stop_) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(listener_, &read_set);
        timeval tv{0, 50000};
        if (select(listener_ + 1, &read_set, nullptr, nullptr, &tv) <= 0) continue;
        int fd = accept(listener_, nullptr, nullptr);
        if (fd < 0) continue;
        Serve(fd);
        close(fd);
    }
}

void TestTlsServer::Serve(int fd) {
    auto tls = std::make_unique<ServerTls>();
    mbedtls_entropy_add_source(&tls->entropy, EntropySource, nullptr, 32, MBEDTLS_ENTROPY_SOURCE_STRONG);
    mbedtls_ctr_drbg_seed(&tls->drbg, mbedtls_entropy_func, &tls->entropy, nullptr, 0);
    mbedtls_x509_crt_parse(&tls->cert, reinterpret_cast<const unsigned char *>(cert_.c_str()), cert_.size() + 1);
    mbedtls_pk_parse_key(&tls->key, reinterpret_cast<const unsigned char *>(key_.c_str()), key_.size() + 1, nullptr, 0,
                         mbedtls_ctr_drbg_random, &tls->drbg);
    mbedtls_ssl_config_defaults(&tls->conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM,
                                MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_rng(&tls->conf, mbedtls_ctr_drbg_random, &tls->drbg);
    mbedtls_ssl_conf_own_cert(&tls->conf, &tls->cert, &tls->key);
    mbedtls_ssl_setup(&tls->ssl, &tls->conf);
    mbedtls_ssl_set_bio(&tls->ssl, &fd, SendCb, RecvCb, nullptr);
    if (mbedtls_ssl_handshake(&tls->ssl) != 0) return;

    std::string request;
    unsigned char buf[4096];
    while (request.find("\r\n\r\n") == std::string::npos) {
        int n = mbedtls_ssl_read(&tls->ssl, buf, sizeof buf);
        if (n <= 0) return;
        request.append(reinterpret_cast<char *>(buf), size_t(n));
    }
    size_t head_end = request.find("\r\n\r\n") + 4;
    size_t length = size_t(std::atol(Field(request, "Content-Length: ").c_str()));
    while (request.size() < head_end + length) {
        int n = mbedtls_ssl_read(&tls->ssl, buf, sizeof buf);
        if (n <= 0) return;
        request.append(reinterpret_cast<char *>(buf), size_t(n));
    }

    std::string response;
    if (mode_ == Mode::Silent) {
        while (!stop_) usleep(20000);
        return;
    }
    if (mode_ == Mode::Chunked) {
        response = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nabc\r\n3\r\ndef\r\n0\r\n\r\n";
    } else {
        std::string method = request.substr(0, request.find(' '));
        std::string body = "{\"method\":\"" + method + "\",\"auth\":\"" + Field(request, "Authorization: ") +
                           "\",\"body\":\"" + request.substr(head_end, length) + "\"}";
        response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.size()) +
                   "\r\n\r\n" + body;
    }
    size_t sent = 0;
    while (sent < response.size()) {
        int n = mbedtls_ssl_write(&tls->ssl, reinterpret_cast<const unsigned char *>(response.data()) + sent,
                                  response.size() - sent);
        if (n <= 0) return;
        sent += size_t(n);
    }
    mbedtls_ssl_close_notify(&tls->ssl);
}

}
