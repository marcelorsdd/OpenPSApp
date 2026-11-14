#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <chrono>
#include <thread>

#include "helpers.hpp"
#include "http/client.hpp"
#include "http/server.hpp"
#include "test.hpp"

using namespace openpsapp;

namespace {
using Clock = std::chrono::steady_clock;

int64_t MillisSince(Clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
}

int Connect(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    timeval tv{5, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

bool SendAll(int fd, const std::string &data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

std::string ReadAll(int fd) {
    std::string raw;
    char buf[1024];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) raw.append(buf, static_cast<size_t>(n));
    return raw;
}

std::string Exchange(uint16_t port, const std::string &request) {
    int fd = Connect(port);
    if (fd < 0) return "";
    std::string raw = SendAll(fd, request) ? ReadAll(fd) : "";
    close(fd);
    return raw;
}

std::string StatusLine(const std::string &raw) { return raw.substr(0, raw.find("\r\n")); }

std::string Body(const std::string &raw) {
    size_t at = raw.find("\r\n\r\n");
    return at == std::string::npos ? "" : raw.substr(at + 4);
}

int64_t MillisUntilDropped(uint16_t port, const std::string &head, const std::string &drip) {
    int fd = Connect(port);
    if (fd < 0 || !SendAll(fd, head)) return -1;
    Clock::time_point start = Clock::now();
    bool dropped = false;
    while (!dropped && MillisSince(start) < 3000) {
        usleep(50 * 1000);
        dropped = send(fd, drip.data(), drip.size(), 0) <= 0;
    }
    close(fd);
    return dropped ? MillisSince(start) : -1;
}

struct LiveServer {
    http::Server server;
    std::thread thread;

    explicit LiveServer(const http::Router &router, http::ServerLimits limits = {}) : server(router, limits) {
        std::string err;
        CHECK(server.Listen(0, &err));
        CHECK(server.port() != 0);
        thread = std::thread([this] { server.Serve(); });
    }

    ~LiveServer() {
        server.Stop();
        thread.join();
    }

    uint16_t port() const { return server.port(); }
};

http::Router EchoRouter() {
    http::Router router;
    router.Get("/x", [](const http::Request &) { return http::Json("{}"); });
    router.Post("/echo", [](const http::Request &req) { return http::Json(std::to_string(req.body.size())); });
    return router;
}
}

TEST(target_parsing_decodes_path_and_query) {
    std::string path;
    std::map<std::string, std::string> query;
    http::ParseTarget("/api/v1/debug/file?path=%2Fuser%2Fhome&x=a+b", &path, &query);
    CHECK_EQ(path, std::string("/api/v1/debug/file"));
    CHECK_EQ(query["path"], std::string("/user/home"));
    CHECK_EQ(query["x"], std::string("a b"));
}

TEST(target_parsing_keeps_plus_in_paths_and_stray_percents) {
    std::string path;
    std::map<std::string, std::string> query;
    http::ParseTarget("/games/a+b%20c?q=c%2Bd+e", &path, &query);
    CHECK_EQ(path, std::string("/games/a+b c"));
    CHECK_EQ(query["q"], std::string("c+d e"));

    for (const char *raw : {"/x%", "/x%4", "/x%zz"}) {
        http::ParseTarget(raw, &path, &query);
        CHECK_EQ(path, std::string(raw));
    }
    http::ParseTarget("/x%%41", &path, &query);
    CHECK_EQ(path, std::string("/x%A"));
    http::ParseTarget("/x%41", &path, &query);
    CHECK_EQ(path, std::string("/xA"));
    query.clear();
    http::ParseTarget("/x?a=%&b=50%&c=%4", &path, &query);
    CHECK_EQ(query["a"], std::string("%"));
    CHECK_EQ(query["b"], std::string("50%"));
    CHECK_EQ(query["c"], std::string("%4"));
}

TEST(header_parsing_lowercases_names_and_joins_repeats) {
    std::map<std::string, std::string> headers;
    http::ParseHeaders(
        "POST /x HTTP/1.1\r\nHost: 127.0.0.1:7777\r\nX-OpenPSApp:\t1 \r\nAccept: a\r\naccept: b\r\nbroken\r\n\r\n",
        &headers);
    CHECK_EQ(headers.size(), size_t(3));
    CHECK_EQ(headers["host"], std::string("127.0.0.1:7777"));
    CHECK_EQ(headers["x-openpsapp"], std::string("1"));
    CHECK_EQ(headers["accept"], std::string("a, b"));
}

TEST(loopback_means_127_slash_8) {
    CHECK(http::IsLoopback("127.0.0.1"));
    CHECK(http::IsLoopback("127.8.9.10"));
    CHECK(!http::IsLoopback("192.168.1.15"));
    CHECK(!http::IsLoopback("128.0.0.1"));
    CHECK(!http::IsLoopback("0.0.0.0"));
    CHECK(!http::IsLoopback("127.0.0.1.example"));
    CHECK(!http::IsLoopback(""));
}

TEST(router_matches_params_methods_and_misses) {
    http::Router router;
    router.Get("/games/:titleId", [](const http::Request &req) { return http::Json(req.params.at("titleId")); });
    router.Post("/x", [](const http::Request &req) { return http::Json(req.body); });

    CHECK_EQ(test::Get(router, "/games/PPSA01234").body, std::string("PPSA01234"));
    CHECK_EQ(test::Call(router, "HEAD", "/games/PPSA01234").status, 200);
    CHECK_EQ(test::Get(router, "/games").status, 404);
    CHECK_EQ(test::Get(router, "/games").body, std::string(R"({"error":"not_found"})"));
    CHECK_EQ(test::Call(router, "POST", "/games/PPSA01234").status, 405);
    CHECK_EQ(test::Call(router, "POST", "/games/PPSA01234").body, std::string(R"({"error":"method_not_allowed"})"));
    CHECK_EQ(test::Call(router, "POST", "/x").status, 200);
    CHECK_EQ(test::Get(router, "/x").status, 405);
}

TEST(live_server_detects_itself_and_reads_post_bodies) {
    test::App app;
    LiveServer live(app.router);

    CHECK(routes::ServedByOpenPSApp(live.port()));
    CHECK(!routes::ServedByOpenPSApp(live.port() + 1));

    std::string host = "127.0.0.1:" + std::to_string(live.port());
    std::string marked = Exchange(live.port(), "POST /api/v1/games/PPSA05555/unhide HTTP/1.1\r\nHost: " + host +
                                                   "\r\nX-OpenPSApp: 1\r\nContent-Length: 2\r\n\r\n{}");
    CHECK_EQ(StatusLine(marked), std::string("HTTP/1.1 200 OK"));
    std::string unmarked = Exchange(live.port(), "POST /api/v1/games/PPSA05555/unhide HTTP/1.1\r\nHost: " + host +
                                                     "\r\nContent-Length: 2\r\n\r\n{}");
    CHECK_EQ(StatusLine(unmarked), std::string("HTTP/1.1 403 Forbidden"));
}

TEST(live_server_hands_peer_and_headers_to_handlers_without_cors) {
    http::Router router;
    router.Get("/echo", [](const http::Request &req) {
        return http::Json(req.peer + "|" + http::Header(req, "host") + "|" + http::Header(req, "x-openpsapp"));
    });
    LiveServer live(router);
    std::string host = "127.0.0.1:" + std::to_string(live.port());

    std::string raw = Exchange(live.port(), "GET /echo HTTP/1.1\r\nHost: " + host + "\r\nX-OpenPSApp: 1\r\n\r\n");
    CHECK_EQ(StatusLine(raw), std::string("HTTP/1.1 200 OK"));
    CHECK_EQ(Body(raw), "127.0.0.1|" + host + "|1");
    CHECK(raw.find("Access-Control") == std::string::npos);
    std::string missing = Exchange(live.port(), "GET /missing HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
    CHECK(missing.find("404") != std::string::npos);
    CHECK(missing.find("Access-Control") == std::string::npos);
}

TEST(live_server_answers_busy_at_once_when_every_worker_is_taken) {
    http::Router router = EchoRouter();
    LiveServer live(router, {1, 2000});
    int idle = Connect(live.port());
    CHECK(idle >= 0);

    for (int i = 0; i < 3; ++i) {
        Clock::time_point start = Clock::now();
        int fd = Connect(live.port());
        std::string raw = ReadAll(fd);
        close(fd);
        CHECK_EQ(StatusLine(raw), std::string("HTTP/1.1 503 Service Unavailable"));
        CHECK_EQ(Body(raw), std::string(R"({"error":"busy"})"));
        CHECK(MillisSince(start) < 1000);
    }

    close(idle);
    std::string served;
    for (int i = 0; i < 100 && StatusLine(served) != "HTTP/1.1 200 OK"; ++i) {
        usleep(10 * 1000);
        served = Exchange(live.port(), "GET /x HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");
    }
    CHECK_EQ(StatusLine(served), std::string("HTTP/1.1 200 OK"));
}

TEST(live_server_drops_requests_that_trickle_past_the_deadline) {
    http::Router router = EchoRouter();
    LiveServer live(router, {4, 300});

    int64_t headers = MillisUntilDropped(live.port(), "GET /x HTTP/1.1\r\n", "X-Slow: 1\r\n");
    CHECK(headers >= 0);
    CHECK(headers < 2000);

    int64_t body = MillisUntilDropped(live.port(), "POST /echo HTTP/1.1\r\nContent-Length: 1000\r\n\r\n", "a");
    CHECK(body >= 0);
    CHECK(body < 2000);

    CHECK_EQ(StatusLine(Exchange(live.port(), "GET /x HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")),
             std::string("HTTP/1.1 200 OK"));
}

TEST(live_server_limits_header_and_body_sizes) {
    http::Router router = EchoRouter();
    LiveServer live(router);

    std::string head = "GET /x HTTP/1.1\r\nX-Big: ";
    std::string oversized = head + std::string(16 * 1024 + 1 - head.size(), 'a');
    std::string raw = Exchange(live.port(), oversized);
    CHECK_EQ(StatusLine(raw), std::string("HTTP/1.1 431 Request Header Fields Too Large"));
    CHECK_EQ(Body(raw), std::string(R"({"error":"headers_too_large"})"));

    std::string big_header = head + std::string(8 * 1024, 'a') + "\r\nHost: 127.0.0.1\r\n\r\n";
    CHECK_EQ(StatusLine(Exchange(live.port(), big_header)), std::string("HTTP/1.1 200 OK"));

    raw = Exchange(live.port(), "POST /echo HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 65537\r\n\r\n");
    CHECK_EQ(StatusLine(raw), std::string("HTTP/1.1 413 Content Too Large"));
    CHECK_EQ(Body(raw), std::string(R"({"error":"too_large"})"));

    raw = Exchange(live.port(),
                   "POST /echo HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 65536\r\n\r\n" + std::string(65536, 'b'));
    CHECK_EQ(StatusLine(raw), std::string("HTTP/1.1 200 OK"));
    CHECK_EQ(Body(raw), std::string("65536"));
}

TEST(allowed_host_accepts_loopback_and_ip_literals_only) {
    CHECK(http::IsAllowedHost("127.0.0.1"));
    CHECK(http::IsAllowedHost("127.0.0.1:7777"));
    CHECK(http::IsAllowedHost("localhost"));
    CHECK(http::IsAllowedHost("localhost:7777"));
    CHECK(http::IsAllowedHost("192.168.1.15:7777"));
    CHECK(http::IsAllowedHost("10.0.0.5"));
    CHECK(http::IsAllowedHost("[::1]"));
    CHECK(http::IsAllowedHost("[::1]:7777"));
    CHECK(http::IsAllowedHost("[fe80::1]:7777"));

    CHECK(!http::IsAllowedHost(""));
    CHECK(!http::IsAllowedHost("attacker.com"));
    CHECK(!http::IsAllowedHost("attacker.com:7777"));
    CHECK(!http::IsAllowedHost("console.local"));
    CHECK(!http::IsAllowedHost("127.0.0.1.attacker.com"));
    CHECK(!http::IsAllowedHost("localhost.attacker.com"));
    CHECK(!http::IsAllowedHost("127.0.0.1:"));
    CHECK(!http::IsAllowedHost("127.0.0.1:0"));
    CHECK(!http::IsAllowedHost("127.0.0.1:99999"));
    CHECK(!http::IsAllowedHost("127.0.0.1:7x7"));
    CHECK(!http::IsAllowedHost("[::1"));
}

TEST(live_server_rejects_rebinding_hosts) {
    http::Router router = EchoRouter();
    LiveServer live(router);
    std::string port = std::to_string(live.port());

    for (const std::string &host :
         {std::string("attacker.com"), "attacker.com:" + port, std::string("console.local")}) {
        std::string raw = Exchange(live.port(), "GET /x HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
        CHECK_EQ(StatusLine(raw), std::string("HTTP/1.1 403 Forbidden"));
        CHECK_EQ(Body(raw), std::string(R"({"error":"bad_host"})"));
    }

    std::string no_host = Exchange(live.port(), "GET /x HTTP/1.1\r\n\r\n");
    CHECK_EQ(StatusLine(no_host), std::string("HTTP/1.1 403 Forbidden"));

    for (const std::string &host : {std::string("127.0.0.1"), "127.0.0.1:" + port, "localhost:" + port,
                                    "192.168.1.15:" + port, "[::1]:" + port}) {
        std::string raw = Exchange(live.port(), "GET /x HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
        CHECK_EQ(StatusLine(raw), std::string("HTTP/1.1 200 OK"));
    }
}
