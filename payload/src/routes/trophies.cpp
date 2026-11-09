#include <cstdint>
#include <cstdlib>

#include "routes/routes.hpp"
#include "routes/views.hpp"

namespace playtrack::routes {
namespace {
size_t EarnedLimit(const http::Request &req) {
    auto it = req.query.find("limit");
    unsigned long n = it == req.query.end() ? 0 : std::strtoul(it->second.c_str(), nullptr, 10);
    return n == 0 ? SIZE_MAX : n;
}
}

void RegisterTrophies(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/trophies/earned", [&ctx](const http::Request &req) {
        auto lib = CurrentLibrary(ctx.language);
        JsonWriter w;
        w.BeginArray();
        for (const EarnedTrophy &e : lib->EarnedTrophies(EarnedLimit(req))) {
            auto title = lib->title_for_set.find(e.set->id);
            w.BeginObject()
                .Key("setId").String(e.set->id)
                .Key("setName").String(e.set->name)
                .Key("titleId").StringOrNull(title != lib->title_for_set.end() ? title->second : "")
                .Key("trophy");
            WriteTrophy(w, *e.trophy);
            w.EndObject();
        }
        w.EndArray();
        return http::Json(w.take());
    });

    router.Get("/api/v1/trophy-sets", [&ctx](const http::Request &) {
        auto lib = CurrentLibrary(ctx.language);
        JsonWriter w;
        w.BeginArray();
        for (const TrophySet &set : lib->trophies.sets) WriteTrophySet(w, *lib, set);
        w.EndArray();
        return http::Json(w.take());
    });

    router.Get("/api/v1/trophy-sets/:id", [&ctx](const http::Request &req) {
        auto lib = CurrentLibrary(ctx.language);
        const TrophySet *set = lib->FindSet(req.params.at("id"));
        if (!set) return http::Error(404, "trophy set not found");
        JsonWriter w;
        WriteTrophySet(w, *lib, *set);
        return http::Json(w.take());
    });

    router.Get("/api/v1/trophy-sets/:id/trophies", [&ctx](const http::Request &req) {
        auto lib = CurrentLibrary(ctx.language);
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
