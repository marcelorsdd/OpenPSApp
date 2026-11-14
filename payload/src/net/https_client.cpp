#include "net/https_client.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

#include "net/url.hpp"

namespace openpsapp::net {
namespace {
constexpr size_t kMaxResponse = 8u << 20;
constexpr size_t kReadChunk = 16 * 1024;

struct Tls {
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt ca;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;

    Tls() {
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&ca);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&drbg);
    }
    ~Tls() {
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_config_free(&conf);
        mbedtls_x509_crt_free(&ca);
        mbedtls_ctr_drbg_free(&drbg);
        mbedtls_entropy_free(&entropy);
    }
};

struct Socket {
    int fd = -1;
    ~Socket() {
        if (fd >= 0) close(fd);
    }
};

int EntropySource(void *, unsigned char *out, size_t len, size_t *olen) {
    arc4random_buf(out, len);
    *olen = len;
    return 0;
}

int SendCb(void *ctx, const unsigned char *buf, size_t len) {
    ssize_t n = send(*static_cast<int *>(ctx), buf, len, 0);
    if (n >= 0) return int(n);
    return errno == EAGAIN || errno == EWOULDBLOCK ? MBEDTLS_ERR_SSL_TIMEOUT : MBEDTLS_ERR_NET_SEND_FAILED;
}

int RecvCb(void *ctx, unsigned char *buf, size_t len) {
    ssize_t n = recv(*static_cast<int *>(ctx), buf, len, 0);
    if (n >= 0) return int(n);
    return errno == EAGAIN || errno == EWOULDBLOCK ? MBEDTLS_ERR_SSL_TIMEOUT : MBEDTLS_ERR_NET_RECV_FAILED;
}

std::string Hex(int code) {
    char buf[16];
    std::snprintf(buf, sizeof buf, "-0x%04x", unsigned(-code));
    return buf;
}

bool ConnectWithTimeout(const Url &url, int timeout_ms, Socket *sock, std::string *err) {
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *res = nullptr;
    std::string port = std::to_string(url.port);
    if (getaddrinfo(url.host.c_str(), port.c_str(), &hints, &res) != 0 || !res) {
        *err = "dns: cannot resolve " + url.host;
        return false;
    }
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> guard(res, freeaddrinfo);
    for (addrinfo *ai = res; ai; ai = ai->ai_next) {
        Socket s;
        s.fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (s.fd < 0) continue;
#ifdef SO_NOSIGPIPE
        int one = 1;
        setsockopt(s.fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof one);
#endif
        int flags = fcntl(s.fd, F_GETFL, 0);
        fcntl(s.fd, F_SETFL, flags | O_NONBLOCK);
        int rc = connect(s.fd, ai->ai_addr, ai->ai_addrlen);
        if (rc != 0 && errno != EINPROGRESS) continue;
        if (rc != 0) {
            fd_set write_set;
            FD_ZERO(&write_set);
            FD_SET(s.fd, &write_set);
            timeval tv{timeout_ms / 1000, (timeout_ms % 1000) * 1000};
            if (select(s.fd + 1, nullptr, &write_set, nullptr, &tv) <= 0) {
                *err = "timeout: connect";
                continue;
            }
            int so_error = 0;
            socklen_t len = sizeof so_error;
            getsockopt(s.fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
            if (so_error != 0) continue;
        }
        fcntl(s.fd, F_SETFL, flags);
        timeval io{timeout_ms / 1000, (timeout_ms % 1000) * 1000};
        setsockopt(s.fd, SOL_SOCKET, SO_RCVTIMEO, &io, sizeof io);
        setsockopt(s.fd, SOL_SOCKET, SO_SNDTIMEO, &io, sizeof io);
        sock->fd = s.fd;
        s.fd = -1;
        err->clear();
        return true;
    }
    if (err->empty()) *err = "connect: " + url.host;
    return false;
}

std::string HandshakeError(Tls &tls, int rc) {
    if (rc == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
        char info[512];
        mbedtls_x509_crt_verify_info(info, sizeof info, "", mbedtls_ssl_get_verify_result(&tls.ssl));
        std::string text(info);
        while (!text.empty() && (text.back() == '\n' || text.back() == ' ')) text.pop_back();
        for (size_t at = text.find('\n'); at != std::string::npos; at = text.find('\n', at)) text.replace(at, 1, "; ");
        return "certificate: " + text;
    }
    if (rc == MBEDTLS_ERR_SSL_TIMEOUT) return "timeout: handshake";
    return "tls: handshake " + Hex(rc);
}
}

HttpsResponse HttpsSend(const HttpsRequest &request, const std::vector<std::string_view> &ca_pems) {
    HttpsResponse out;
    std::optional<Url> url = ParseHttpsUrl(request.url);
    if (!url) {
        out.error = "url: only https:// URLs are allowed";
        return out;
    }

    auto tls = std::make_unique<Tls>();
    mbedtls_entropy_add_source(&tls->entropy, EntropySource, nullptr, 32, MBEDTLS_ENTROPY_SOURCE_STRONG);
    static const char kPers[] = "openpsapp-https";
    if (mbedtls_ctr_drbg_seed(&tls->drbg, mbedtls_entropy_func, &tls->entropy,
                              reinterpret_cast<const unsigned char *>(kPers), sizeof kPers - 1) != 0) {
        out.error = "tls: random generator";
        return out;
    }
    for (std::string_view pem : ca_pems) {
        std::string copy(pem);
        mbedtls_x509_crt_parse(&tls->ca, reinterpret_cast<const unsigned char *>(copy.c_str()), copy.size() + 1);
    }
    mbedtls_ssl_config_defaults(&tls->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_authmode(&tls->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&tls->conf, &tls->ca, nullptr);
    mbedtls_ssl_conf_rng(&tls->conf, mbedtls_ctr_drbg_random, &tls->drbg);
    if (mbedtls_ssl_setup(&tls->ssl, &tls->conf) != 0 || mbedtls_ssl_set_hostname(&tls->ssl, url->host.c_str()) != 0) {
        out.error = "tls: setup";
        return out;
    }

    Socket sock;
    if (!ConnectWithTimeout(*url, request.timeout_ms, &sock, &out.error)) return out;
    mbedtls_ssl_set_bio(&tls->ssl, &sock.fd, SendCb, RecvCb, nullptr);

    int rc;
    while ((rc = mbedtls_ssl_handshake(&tls->ssl)) != 0) {
        if (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        out.error = HandshakeError(*tls, rc);
        return out;
    }

    std::string wire = BuildRequest(request.method, *url, request.headers, request.body);
    size_t sent = 0;
    while (sent < wire.size()) {
        rc = mbedtls_ssl_write(&tls->ssl, reinterpret_cast<const unsigned char *>(wire.data()) + sent, wire.size() - sent);
        if (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (rc <= 0) {
            out.error = rc == MBEDTLS_ERR_SSL_TIMEOUT ? "timeout: send" : "tls: send " + Hex(rc);
            return out;
        }
        sent += size_t(rc);
    }

    std::string raw;
    std::vector<unsigned char> chunk(kReadChunk);
    while (true) {
        rc = mbedtls_ssl_read(&tls->ssl, chunk.data(), chunk.size());
        if (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (rc == 0 || rc == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) break;
        if (rc < 0) {
            out.error = rc == MBEDTLS_ERR_SSL_TIMEOUT ? "timeout: receive" : "tls: receive " + Hex(rc);
            return out;
        }
        raw.append(reinterpret_cast<const char *>(chunk.data()), size_t(rc));
        if (raw.size() > kMaxResponse) {
            out.error = "response: too large";
            return out;
        }
    }
    mbedtls_ssl_close_notify(&tls->ssl);

    std::string err;
    if (!ParseResponse(raw, &out.status, &out.body, &err)) {
        out.status = 0;
        out.error = "response: " + err;
    }
    return out;
}

}
