#pragma once

#include <map>
#include <string>

#include "http/http.hpp"
#include "platform/platform.hpp"
#include "routes/routes.hpp"

namespace test {
inline openpsapp::http::Response Call(const openpsapp::http::Router &router, const std::string &method,
                                      const std::string &target,
                                      const std::map<std::string, std::string> &headers = {}) {
    openpsapp::http::Request req;
    req.method = method;
    req.headers = headers;
    openpsapp::http::ParseTarget(target, &req.path, &req.query);
    return router.Dispatch(req);
}

inline openpsapp::http::Response AppPost(const openpsapp::http::Router &router, const std::string &target) {
    return Call(router, "POST", target, {{"x-openpsapp", "1"}});
}

inline openpsapp::http::Response Get(const openpsapp::http::Router &router, const std::string &target) {
    return Call(router, "GET", target);
}

struct App {
    openpsapp::routes::Context ctx;
    openpsapp::http::Router router;

    explicit App(bool debug = false) {
        ctx.version = "test";
        ctx.data_dir = openpsapp::platform::Root() + "/data/openpsapp";
        ctx.config.debug = debug;
        ctx.language = "pt-BR";
        openpsapp::routes::RegisterAll(router, ctx);
    }
};

}
