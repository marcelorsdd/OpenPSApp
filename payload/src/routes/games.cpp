#include "routes/routes.hpp"
#include "routes/views.hpp"
#include "services/hidden.hpp"

namespace openpsapp::routes {
namespace {
http::Response GameJson(const Library &lib, const GamePlaytime &game) {
    JsonWriter w;
    WriteGame(w, lib, game);
    return http::Json(w.take());
}
}

void RegisterGames(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/games", [&ctx](const http::Request &) {
        auto lib = CurrentLibrary(ctx.language);
        if (!lib->playtime.error.empty() && lib->playtime.games.empty()) return http::Error(500, lib->playtime.error);
        JsonWriter w;
        w.BeginArray();
        for (const GamePlaytime &g : lib->playtime.games) WriteGame(w, *lib, g);
        w.EndArray();
        return http::Json(w.take());
    });

    router.Get("/api/v1/games/:titleId", [&ctx](const http::Request &req) {
        auto lib = CurrentLibrary(ctx.language);
        const GamePlaytime *game = lib->FindGame(req.params.at("titleId"));
        return game ? GameJson(*lib, *game) : http::Error(404, "game not found");
    });

    for (bool hide : {true, false}) {
        router.Post(hide ? "/api/v1/games/:titleId/hide" : "/api/v1/games/:titleId/unhide",
                    [&ctx, hide](const http::Request &req) {
                        auto lib = CurrentLibrary(ctx.language);
                        const GamePlaytime *game = lib->FindGame(req.params.at("titleId"));
                        if (!game) return http::Error(404, "game not found");
                        if (!hidden::Set(game->title_id, hide)) return http::Error(500, "could not save hidden.json");
                        return GameJson(*lib, *game);
                    });
    }
}

}
