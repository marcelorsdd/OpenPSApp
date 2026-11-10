#pragma once

#include <string>

#include "http/http.hpp"
#include "platform/platform.hpp"
#include "routes/routes.hpp"

namespace test {
inline openpsapp::http::Response Call(const openpsapp::http::Router &router, const std::string &method,
                                      const std::string &target) {
    openpsapp::http::Request req;
    req.method = method;
    openpsapp::http::ParseTarget(target, &req.path, &req.query);
    return router.Dispatch(req);
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
