#include "routes/routes.hpp"
#include "routes/views.hpp"

namespace openpsapp::routes {
void RegisterProfile(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/profile", [&ctx](const http::Request &) {
        auto lib = CurrentLibrary(ctx.language);
        ProfileTotals totals = lib->Totals();

        JsonWriter w;
        w.BeginObject().Key("user");
        WriteUser(w, lib->user);
        w.BeginObject("totals")
            .Int("games", totals.games)
            .Int("playtimeSeconds", totals.playtime_seconds)
            .BeginObject("trophies");
        for (size_t i = 0; i < kGrades.size(); ++i) w.Int(GradeName(kGrades[i]), totals.trophies.by_grade[i].earned);
        w.Int("earned", totals.trophies.earned).Int("total", totals.trophies.total).EndObject();
        w.EndObject().EndObject();
        return http::Json(w.take());
    });
}

}
