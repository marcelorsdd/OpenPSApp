#include "routes/routes.hpp"
#include "routes/views.hpp"

namespace playtrack::routes {
void RegisterTrophies(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/trophy-sets", [&ctx](const http::Request &) {
        auto lib = CurrentLibrary(ctx.config.language);
        JsonWriter w;
        w.BeginArray();
        for (const TrophySet &set : lib->trophies.sets) WriteTrophySet(w, *lib, set);
        w.EndArray();
        return http::Json(w.take());
    });

    router.Get("/api/v1/trophy-sets/:id", [&ctx](const http::Request &req) {
        auto lib = CurrentLibrary(ctx.config.language);
        const TrophySet *set = lib->FindSet(req.params.at("id"));
        if (!set) return http::Error(404, "trophy set not found");
        JsonWriter w;
        WriteTrophySet(w, *lib, *set);
        return http::Json(w.take());
    });

    router.Get("/api/v1/trophy-sets/:id/trophies", [&ctx](const http::Request &req) {
        auto lib = CurrentLibrary(ctx.config.language);
        const TrophySet *set = lib->FindSet(req.params.at("id"));
        if (!set) return http::Error(404, "trophy set not found");
        JsonWriter w;
        w.BeginArray();
        for (const Trophy &t : set->trophies) WriteTrophy(w, t);
        w.EndArray();
        return http::Json(w.take());
    });
}

}
