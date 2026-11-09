#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "helpers.hpp"
#include "http/client.hpp"
#include "http/server.hpp"
#include "test.hpp"
#include "util/thread.hpp"

using namespace playtrack;

namespace {
std::string StatusLine(uint16_t port, const std::string &request) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    std::string raw;
    if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0 &&
        send(fd, request.data(), request.size(), 0) > 0) {
        char buf[1024];
        ssize_t n;
        while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) raw.append(buf, static_cast<size_t>(n));
    }
    close(fd);
    return raw.substr(0, raw.find("\r\n"));
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

TEST(router_matches_params_methods_and_misses) {
    http::Router router;
    router.Get("/games/:titleId", [](const http::Request &req) { return http::Json(req.params.at("titleId")); });
    router.Post("/x", [](const http::Request &req) { return http::Json(req.body); });

    CHECK_EQ(test::Get(router, "/games/PPSA01234").body, std::string("PPSA01234"));
    CHECK_EQ(test::Call(router, "HEAD", "/games/PPSA01234").status, 200);
    CHECK_EQ(test::Get(router, "/games").status, 404);
    CHECK_EQ(test::Call(router, "POST", "/games/PPSA01234").status, 405);
    CHECK_EQ(test::Call(router, "POST", "/x").status, 200);
    CHECK_EQ(test::Get(router, "/x").status, 405);
}

TEST(live_server_detects_itself_and_reads_post_bodies) {
    static test::App app;
    static http::Server server(app.router);
    constexpr uint16_t kPort = 47777;
    std::string err;
    CHECK(server.Listen(kPort, &err));
    Spawn([] { server.Serve(); });

    CHECK(routes::ServedByPlayTrack(kPort));
    CHECK(!routes::ServedByPlayTrack(kPort + 1));

    std::string status = StatusLine(
        kPort, "POST /api/v1/games/PPSA05555/unhide HTTP/1.1\r\nHost: x\r\nContent-Length: 2\r\n\r\n{}");
    CHECK_EQ(status, std::string("HTTP/1.1 200 OK"));
}
