#include "routes/views.hpp"

#include "services/hidden.hpp"
#include "services/icons.hpp"
#include "util/timeutil.hpp"

namespace playtrack::routes {
void WriteUser(JsonWriter &w, const std::optional<platform::User> &user) {
    if (!user) {
        w.Null();
        return;
    }
    w.BeginObject().Key("id").String(user->id_hex).Key("name").String(user->name).EndObject();
}

void WriteGame(JsonWriter &w, const Library &lib, const GamePlaytime &game) {
    auto set = lib.set_for_title.find(game.title_id);
    w.BeginObject()
        .Key("titleId").String(game.title_id)
        .Key("name").String(game.name)
        .Key("playtimeSeconds").Int(game.playtime_seconds)
        .Key("lastPlayedAt").StringOrNull(game.last_played_at)
        .Key("iconUrl").StringOrNull(icons::Url(game.icon))
        .Key("trophySetId").StringOrNull(set != lib.set_for_title.end() ? set->second : "")
        .Key("hidden").Bool(hidden::Contains(game.title_id))
        .EndObject();
}

void WriteGradeCounts(JsonWriter &w, const Tally &tally) {
    w.BeginObject();
    for (size_t i = 0; i < kGrades.size(); ++i) {
        const GradeTally &g = tally.by_grade[i];
        w.Key(GradeName(kGrades[i])).BeginObject().Key("earned").Int(g.earned).Key("total").Int(g.total).EndObject();
    }
    w.EndObject();
}

void WriteTrophySet(JsonWriter &w, const Library &lib, const TrophySet &set) {
    Tally tally = set.Count();
    auto title = lib.title_for_set.find(set.id);
    w.BeginObject()
        .Key("id").String(set.id)
        .Key("name").String(set.name)
        .Key("titleId").StringOrNull(title != lib.title_for_set.end() ? title->second : "")
        .Key("iconUrl").StringOrNull(icons::Url(set.icon))
        .Key("progress").Int(set.Progress())
        .Key("earned").Int(tally.earned)
        .Key("total").Int(tally.total)
        .Key("lastEarnedAt").StringOrNull(timeutil::FormatIsoOrEmpty(set.LastEarnedAt()))
        .Key("counts");
    WriteGradeCounts(w, tally);
    w.EndObject();
}

void WriteTrophy(JsonWriter &w, const Trophy &t) {
    w.BeginObject()
        .Key("id").Int(t.id)
        .Key("name").String(t.name)
        .Key("description").String(t.detail)
        .Key("grade").String(GradeName(t.grade))
        .Key("hidden").Bool(t.hidden)
        .Key("earned").Bool(t.earned)
        .Key("earnedAt").StringOrNull(timeutil::FormatIsoOrEmpty(t.earned_at))
        .Key("iconUrl").StringOrNull(icons::Url(t.icon))
        .EndObject();
}

}
