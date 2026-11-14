#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

#include "http/http.hpp"

using namespace openpsapp::http;

namespace {
const Router &Routes() {
    static const Router router = [] {
        Router r;
        Router::Handler ok = [](const Request &) { return Json("{}"); };
        r.Get("/api/v1/health", ok);
        r.Get("/api/v1/games", ok);
        r.Get("/api/v1/games/:id", ok);
        r.Post("/api/v1/games/:id/hide", ok);
        r.Get("/api/v1/trophies/:set", ok);
        r.Get("/api/v1/icons/:id", ok);
        r.Get("/api/v1/debug/:name", ok);
        r.Get("/:file", ok);
        return r;
    }();
    return router;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string_view input(reinterpret_cast<const char *>(data), size);

    std::string path;
    std::map<std::string, std::string> query;
    ParseTarget(input, &path, &query);

    std::map<std::string, std::string> headers;
    ParseHeaders(input, &headers);

    Request req;
    req.method = "GET";
    req.path = path;
    req.query = query;
    req.headers = headers;
    Response res = Routes().Dispatch(req);
    volatile int sink = res.status + static_cast<int>(res.body.size());
    (void)sink;
    return 0;
}
