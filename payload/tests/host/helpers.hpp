#pragma once

#include <string>

#include "http/http.hpp"
#include "platform/platform.hpp"
#include "routes/routes.hpp"

namespace test {
inline playtrack::http::Response Call(const playtrack::http::Router &router, const std::string &method,
                                      const std::string &target) {
    playtrack::http::Request req;
    req.method = method;
    playtrack::http::ParseTarget(target, &req.path, &req.query);
    return router.Dispatch(req);
}

inline playtrack::http::Response Get(const playtrack::http::Router &router, const std::string &target) {
    return Call(router, "GET", target);
}

struct App {
    playtrack::routes::Context ctx;
    playtrack::http::Router router;

    explicit App(bool debug = false) {
        ctx.version = "test";
        ctx.data_dir = playtrack::platform::Root() + "/data/playtrack";
        ctx.config.debug = debug;
        ctx.language = "pt-BR";
        playtrack::routes::RegisterAll(router, ctx);
    }
};

}
