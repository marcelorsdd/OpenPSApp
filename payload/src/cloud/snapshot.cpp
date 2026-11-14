#include "cloud/snapshot.hpp"

#include "services/hidden.hpp"
#include "util/json.hpp"
#include "util/sha256.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::cloud {
namespace {
std::string AssetId(Snapshot &snap, const icons::Location &where, const AssetHashFn &hash) {
    if (where.path.empty()) return "";
    std::string id = hash(where);
    if (!id.empty()) snap.assets.emplace(id, where);
    return id;
}

std::string SetJson(Snapshot &snap, const TrophySet &set, const AssetHashFn &hash) {
    JsonWriter w;
    w.BeginObject()
        .Key("id").String(set.id)
        .Key("version").String(set.version)
        .Key("name").String(set.name.empty() ? set.id : set.name)
        .Key("iconId").StringOrNull(AssetId(snap, set.icon, hash))
        .Key("trophies").BeginArray();
    for (const Trophy &t : set.trophies) {
        w.BeginObject()
            .Key("id").Int(t.id)
            .Key("name").String(t.name)
            .Key("description").String(t.detail)
            .Key("grade").String(GradeName(t.grade))
            .Key("hidden").Bool(t.hidden)
            .Key("earned").Bool(t.earned)
            .Key("earnedAt").StringOrNull(t.earned ? timeutil::FormatIsoOrEmpty(t.earned_at) : "")
            .Key("iconId").StringOrNull(AssetId(snap, t.icon, hash))
            .EndObject();
    }
    w.EndArray().EndObject();
    return w.take();
}
}

Snapshot BuildSnapshot(const Library &lib, const std::string &profile_name, const std::string &language,
                       const AssetHashFn &hash) {
    Snapshot snap;
    for (const TrophySet &set : lib.trophies.sets) snap.sets[set.id] = SetJson(snap, set, hash);

    JsonWriter w;
    w.BeginObject()
        .Key("profile").BeginObject()
        .Key("name").String(profile_name)
        .Key("language").StringOrNull(language)
        .EndObject()
        .Key("games").BeginArray();
    for (const GamePlaytime &g : lib.playtime.games) {
        auto set = lib.set_for_title.find(g.title_id);
        w.BeginObject()
            .Key("titleId").String(g.title_id)
            .Key("name").String(g.name.empty() ? g.title_id : g.name)
            .Key("playtimeSeconds").Int(g.playtime_seconds)
            .Key("lastPlayedAt").StringOrNull(g.last_played_at)
            .Key("iconId").StringOrNull(AssetId(snap, g.icon, hash))
            .Key("heroId").StringOrNull(AssetId(snap, g.hero, hash))
            .Key("trophySetId").StringOrNull(set != lib.set_for_title.end() ? set->second : "")
            .Key("hidden").Bool(hidden::Contains(g.title_id))
            .EndObject();
    }
    w.EndArray().Key("sets").BeginArray();
    for (const TrophySet &set : lib.trophies.sets) {
        w.BeginObject().Key("id").String(set.id).Key("version").String(set.version).EndObject();
    }
    w.EndArray().Key("assets").BeginArray();
    for (const auto &[id, where] : snap.assets) w.String(id);
    w.EndArray().EndObject();
    snap.manifest = w.take();
    snap.fingerprint = Sha256::Hex(snap.manifest);
    return snap;
}

}
