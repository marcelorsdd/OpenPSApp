#include "routes/routes.hpp"
#include "routes/views.hpp"

namespace playtrack::routes {
void RegisterProfile(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/profile", [&ctx](const http::Request &) {
        auto lib = CurrentLibrary(ctx.config.language);
        ProfileTotals totals = lib->Totals();

        JsonWriter w;
        w.BeginObject().Key("user");
        WriteUser(w, lib->user);
        w.Key("totals").BeginObject()
            .Key("games").Int(totals.games)
            .Key("playtimeSeconds").Int(totals.playtime_seconds)
            .Key("trophies").BeginObject();
        for (size_t i = 0; i < kGrades.size(); ++i) w.Key(GradeName(kGrades[i])).Int(totals.trophies.by_grade[i].earned);
        w.Key("earned").Int(totals.trophies.earned).Key("total").Int(totals.trophies.total).EndObject();
        w.EndObject().EndObject();
        return http::Json(w.take());
    });
}

}
