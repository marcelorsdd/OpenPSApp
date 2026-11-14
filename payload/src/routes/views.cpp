#include "routes/views.hpp"

#include "services/hidden.hpp"
#include "services/icons.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::routes {
void WriteUser(JsonWriter &w, const std::optional<platform::User> &user) {
    if (!user) {
        w.Null();
        return;
    }
    w.BeginObject().String("id", user->id_hex).String("name", user->name).EndObject();
}

void WriteGame(JsonWriter &w, const Library &lib, const GamePlaytime &game) {
    auto set = lib.set_for_title.find(game.title_id);
    w.BeginObject()
        .String("titleId", game.title_id)
        .String("name", game.name)
        .Int("playtimeSeconds", game.playtime_seconds)
        .StringOrNull("lastPlayedAt", game.last_played_at)
        .StringOrNull("iconUrl", icons::Url(game.icon))
        .StringOrNull("heroUrl", icons::Url(game.hero))
        .StringOrNull("trophySetId", set != lib.set_for_title.end() ? set->second : "")
        .Bool("hidden", hidden::Contains(game.title_id))
        .EndObject();
}

void WriteGradeCounts(JsonWriter &w, const Tally &tally) {
    w.BeginObject();
    for (size_t i = 0; i < kGrades.size(); ++i) {
        const GradeTally &g = tally.by_grade[i];
        w.BeginObject(GradeName(kGrades[i])).Int("earned", g.earned).Int("total", g.total).EndObject();
    }
    w.EndObject();
}

void WriteTrophySet(JsonWriter &w, const Library &lib, const TrophySet &set) {
    Tally tally = set.Count();
    auto title = lib.title_for_set.find(set.id);
    w.BeginObject()
        .String("id", set.id)
        .String("name", set.name)
        .StringOrNull("titleId", title != lib.title_for_set.end() ? title->second : "")
        .StringOrNull("iconUrl", icons::Url(set.icon))
        .Int("progress", set.Progress())
        .Int("earned", tally.earned)
        .Int("total", tally.total)
        .StringOrNull("lastEarnedAt", timeutil::FormatIsoOrEmpty(set.LastEarnedAt()))
        .Key("counts");
    WriteGradeCounts(w, tally);
    w.EndObject();
}

void WriteTrophy(JsonWriter &w, const Trophy &t) {
    w.BeginObject()
        .Int("id", t.id)
        .String("name", t.name)
        .String("description", t.detail)
        .String("grade", GradeName(t.grade))
        .Bool("hidden", t.hidden)
        .Bool("earned", t.earned)
        .StringOrNull("earnedAt", timeutil::FormatIsoOrEmpty(t.earned_at))
        .StringOrNull("iconUrl", icons::Url(t.icon))
        .EndObject();
}

}
