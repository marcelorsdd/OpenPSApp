#include "routes/routes.hpp"
#include "services/icons.hpp"
#include "services/library.hpp"

namespace openpsapp::routes {
void RegisterIcons(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/icons/:id", [&ctx](const http::Request &req) {
        const std::string &id = req.params.at("id");
        auto where = icons::Find(id);
        if (!where) {
            CurrentLibrary(ctx.language);
            where = icons::Find(id);
        }
        if (!where) return http::Error(404, "icon not found");
        http::Response res = http::File(where->path, "image/png", where->offset, where->length);
        res.cache_control = http::kCacheForever;
        return res;
    });
}

}
