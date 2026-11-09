#include <algorithm>
#include <cstdlib>

#include "routes/routes.hpp"
#include "routes/views.hpp"

namespace playtrack::routes {
namespace {
constexpr size_t kRecentDefault = 20;
constexpr size_t kRecentMax = 100;

size_t RecentLimit(const http::Request &req) {
    auto it = req.query.find("limit");
    if (it == req.query.end()) return kRecentDefault;
    unsigned long n = std::strtoul(it->second.c_str(), nullptr, 10);
    return n == 0 ? kRecentDefault : std::min<size_t>(n, kRecentMax);
}
}

void RegisterTrophies(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/trophies/recent", [&ctx](const http::Request &req) {
        auto lib = CurrentLibrary(ctx.language);
        JsonWriter w;
        w.BeginArray();
        for (const RecentTrophy &r : lib->RecentTrophies(RecentLimit(req))) {
            auto title = lib->title_for_set.find(r.set->id);
            w.BeginObject()
                .Key("setId").String(r.set->id)
                .Key("setName").String(r.set->name)
                .Key("titleId").StringOrNull(title != lib->title_for_set.end() ? title->second : "")
                .Key("trophy");
            WriteTrophy(w, *r.trophy);
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
