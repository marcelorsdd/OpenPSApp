#include "routes/routes.hpp"

namespace openpsapp::routes {
void RegisterAll(http::Router &router, const Context &ctx) {
    RegisterHealth(router, ctx);
    RegisterProfile(router, ctx);
    RegisterGames(router, ctx);
    RegisterTrophies(router, ctx);
    RegisterIcons(router, ctx);
    RegisterDebug(router, ctx);
    RegisterWeb(router, ctx);
}

}
