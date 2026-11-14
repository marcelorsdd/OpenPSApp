#include <chrono>

#include "net/ca_roots.hpp"
#include "net/http_message.hpp"
#include "net/https_client.hpp"
#include "net/url.hpp"
#include "test.hpp"
#include "tls_fixture.hpp"

using namespace openpsapp;
using namespace openpsapp::net;

TEST(https_urls_parse_host_port_and_target) {
    auto u = ParseHttpsUrl("https://api.openpsapp.app/api/v1/sync/manifest?x=1");
    CHECK(u.has_value());
    if (u) {
        CHECK_EQ(u->host, std::string("api.openpsapp.app"));
        CHECK_EQ(int(u->port), 443);
        CHECK_EQ(u->target, std::string("/api/v1/sync/manifest?x=1"));
    }
    auto p = ParseHttpsUrl("https://localhost:8443");
    CHECK(p.has_value());
    if (p) {
        CHECK_EQ(int(p->port), 8443);
        CHECK_EQ(p->target, std::string("/"));
    }
    CHECK(!ParseHttpsUrl("http://example.com/").has_value());
    CHECK(!ParseHttpsUrl("https:///path").has_value());
    CHECK(!ParseHttpsUrl("https://host:99999/").has_value());
    CHECK(!ParseHttpsUrl("https://host:abc/").has_value());
}

TEST(requests_carry_host_length_and_headers) {
    auto u = ParseHttpsUrl("https://localhost:8443/api?a=b");
    std::string req = BuildRequest("POST", *u, {{"Authorization", "Bearer x"}}, "{}");
    CHECK_EQ(req, std::string("POST /api?a=b HTTP/1.1\r\nHost: localhost:8443\r\nConnection: close\r\n"
                              "Authorization: Bearer x\r\nContent-Length: 2\r\n\r\n{}"));
    auto d = ParseHttpsUrl("https://example.com/");
    CHECK_EQ(BuildRequest("GET", *d, {}, ""),
             std::string("GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));
}

TEST(responses_parse_content_length_chunked_and_close) {
    int status = 0;
    std::string body, err;
    CHECK(ParseResponse("HTTP/1.1 200 OK\r\nContent-Length: 5\r\nX: y\r\n\r\nhello", &status, &body, &err));
    CHECK_EQ(status, 200);
    CHECK_EQ(body, std::string("hello"));

    CHECK(ParseResponse(
        "HTTP/1.1 201 Created\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5;ext=1\r\npedia\r\n0\r\nT: 1\r\n\r\n",
        &status, &body, &err));
    CHECK_EQ(status, 201);
    CHECK_EQ(body, std::string("Wikipedia"));

    CHECK(ParseResponse("HTTP/1.1 204 No Content\r\n\r\n", &status, &body, &err));
    CHECK_EQ(status, 204);
    CHECK_EQ(body, std::string(""));

    CHECK(ParseResponse("HTTP/1.0 200 OK\r\n\r\nuntil close", &status, &body, &err));
    CHECK_EQ(body, std::string("until close"));
}

TEST(responses_reject_truncation_and_garbage) {
    int status = 0;
    std::string body, err;
    CHECK(!ParseResponse("HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nshort", &status, &body, &err));
    CHECK(!ParseResponse("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nab", &status, &body, &err));
    CHECK(!ParseResponse("garbage", &status, &body, &err));
    CHECK(!ParseResponse("HTTP/1.1 200 OK\r\nContent-Length: 1", &status, &body, &err));
    CHECK(!err.empty());
}

TEST(chunked_responses_reject_sizes_past_the_data) {
    const std::string head = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n";
    int status = 0;
    std::string body, err;
    for (const char *size : {"ffffffffffffffff", "fffffffffffffffe", "fffffffffffffffd", "7fffffffffffffff"}) {
        body = "untouched";
        CHECK(!ParseResponse(head + size + "\r\nabc\r\n0\r\n\r\n", &status, &body, &err));
        CHECK_EQ(err, std::string("truncated chunk"));
        CHECK(body.size() < 16);
    }
    CHECK(!ParseResponse(head + "10000000000000000\r\nabc\r\n0\r\n\r\n", &status, &body, &err));
    CHECK_EQ(err, std::string("bad chunk size"));
    CHECK(!ParseResponse(head + "-1\r\nabc\r\n0\r\n\r\n", &status, &body, &err));
    CHECK_EQ(err, std::string("bad chunk size"));
    CHECK(!ParseResponse(head + " 3\r\nabc\r\n0\r\n\r\n", &status, &body, &err));
    CHECK(!ParseResponse(head + "3\r\nabcXY0\r\n\r\n", &status, &body, &err));
    CHECK_EQ(err, std::string("bad chunk terminator"));
    CHECK(!ParseResponse(head + "3\r\nabc", &status, &body, &err));
    CHECK_EQ(err, std::string("truncated chunk"));
}

namespace {
using tls_fixture::TestTlsServer;

std::string LocalUrl(const TestTlsServer &server, const char *host, const char *path) {
    return std::string("https://") + host + ":" + std::to_string(server.port()) + path;
}

bool StartsWith(const std::string &s, const char *prefix) { return s.rfind(prefix, 0) == 0; }
}

TEST(https_round_trips_with_a_trusted_ca) {
    TestTlsServer server(tls_fixture::kServerACert, tls_fixture::kServerAKey, TestTlsServer::Mode::Echo);
    HttpsRequest get{"GET", LocalUrl(server, "localhost", "/hello"), {{"Authorization", "Bearer opsa_t"}}, "", 3000};
    HttpsResponse r = HttpsSend(get, {tls_fixture::kCaA});
    CHECK_EQ(r.error, std::string(""));
    CHECK_EQ(r.status, 200);
    CHECK(r.body.find("\"auth\":\"Bearer opsa_t\"") != std::string::npos);

    HttpsRequest post{
        "POST", LocalUrl(server, "localhost", "/sync"), {{"Content-Type", "application/json"}}, "{\"a\":1}", 3000};
    HttpsResponse p = HttpsSend(post, {tls_fixture::kCaA});
    CHECK(p.ok());
    CHECK(p.body.find("\"method\":\"POST\"") != std::string::npos);
    CHECK(p.body.find("\"body\":\"{\"a\":1}\"") != std::string::npos);
}

TEST(https_rejects_a_certificate_from_an_untrusted_ca) {
    TestTlsServer server(tls_fixture::kServerBCert, tls_fixture::kServerBKey, TestTlsServer::Mode::Echo);
    HttpsResponse r = HttpsSend({"GET", LocalUrl(server, "localhost", "/"), {}, "", 3000}, {tls_fixture::kCaA});
    CHECK(StartsWith(r.error, "certificate"));
    CHECK_EQ(r.status, 0);
}

TEST(https_rejects_a_certificate_for_another_name) {
    TestTlsServer server(tls_fixture::kServerACert, tls_fixture::kServerAKey, TestTlsServer::Mode::Echo);
    HttpsResponse r = HttpsSend({"GET", LocalUrl(server, "127.0.0.1", "/"), {}, "", 3000}, {tls_fixture::kCaA});
    CHECK(StartsWith(r.error, "certificate"));
}

TEST(https_trusts_nothing_without_roots) {
    TestTlsServer server(tls_fixture::kServerACert, tls_fixture::kServerAKey, TestTlsServer::Mode::Echo);
    HttpsResponse r = HttpsSend({"GET", LocalUrl(server, "localhost", "/"), {}, "", 3000}, ProductionRoots());
    CHECK(StartsWith(r.error, "certificate"));
}

TEST(https_times_out_on_a_silent_server) {
    TestTlsServer server(tls_fixture::kServerACert, tls_fixture::kServerAKey, TestTlsServer::Mode::Silent);
    auto start = std::chrono::steady_clock::now();
    HttpsResponse r = HttpsSend({"GET", LocalUrl(server, "localhost", "/"), {}, "", 400}, {tls_fixture::kCaA});
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    CHECK(StartsWith(r.error, "timeout"));
    CHECK(ms < 3000);
}

TEST(https_decodes_chunked_responses) {
    TestTlsServer server(tls_fixture::kServerACert, tls_fixture::kServerAKey, TestTlsServer::Mode::Chunked);
    HttpsResponse r = HttpsSend({"GET", LocalUrl(server, "localhost", "/"), {}, "", 3000}, {tls_fixture::kCaA});
    CHECK(r.ok());
    CHECK_EQ(r.body, std::string("abcdef"));
}

TEST(https_refuses_other_schemes_and_unknown_hosts) {
    CHECK(StartsWith(HttpsSend({"GET", "http://example.com/", {}, "", 1000}, ProductionRoots()).error, "url"));
    CHECK(StartsWith(HttpsSend({"GET", "https://does-not-exist.invalid/", {}, "", 1000}, ProductionRoots()).error,
                     "dns"));
}

TEST(https_refuses_to_run_with_a_broken_ca_certificate) {
    std::string broken(tls_fixture::kCaA);
    broken.replace(broken.find("MIIB"), 4, "AAAA");
    for (std::string_view ca : {std::string_view("not a certificate"), std::string_view(broken)}) {
        HttpsResponse r = HttpsSend({"GET", "https://localhost:9/", {}, "", 1000}, {tls_fixture::kCaA, ca});
        CHECK_EQ(r.error, std::string("tls: bad CA certificate"));
        CHECK_EQ(r.status, 0);
    }
}

TEST(production_roots_are_lets_encrypt_only) {
    auto roots = ProductionRoots();
    CHECK_EQ(roots.size(), size_t(2));
    for (std::string_view pem : roots) CHECK(pem.find("BEGIN CERTIFICATE") != std::string_view::npos);
}
