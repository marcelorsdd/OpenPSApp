#include "routes/routes.hpp"

namespace openpsapp::routes {
void RegisterAll(http::Router &router, const Context &ctx) {
    RegisterHealth(router, ctx);
    RegisterProfile(router, ctx);
    RegisterGames(router, ctx);
    RegisterTrophies(router, ctx);
    RegisterIcons(router, ctx);
    RegisterDebug(router, ctx);
    RegisterCloud(router, ctx);
    RegisterWeb(router, ctx);
}

bool FromApp(const http::Request &req) { return http::Header(req, "x-openpsapp") == "1"; }

}
