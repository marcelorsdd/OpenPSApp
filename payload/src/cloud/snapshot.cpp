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
        .String("id", set.id)
        .String("version", set.version)
        .String("name", set.name.empty() ? set.id : set.name)
        .StringOrNull("iconId", AssetId(snap, set.icon, hash))
        .BeginArray("trophies");
    for (const Trophy &t : set.trophies) {
        w.BeginObject()
            .Int("id", t.id)
            .String("name", t.name)
            .String("description", t.detail)
            .String("grade", GradeName(t.grade))
            .Bool("hidden", t.hidden)
            .Bool("earned", t.earned)
            .StringOrNull("earnedAt", t.earned ? timeutil::FormatIsoOrEmpty(t.earned_at) : "")
            .StringOrNull("iconId", AssetId(snap, t.icon, hash))
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
        .BeginObject("profile")
        .String("name", profile_name)
        .StringOrNull("language", language)
        .EndObject()
        .BeginArray("games");
    for (const GamePlaytime &g : lib.playtime.games) {
        auto set = lib.set_for_title.find(g.title_id);
        w.BeginObject()
            .String("titleId", g.title_id)
            .String("name", g.name.empty() ? g.title_id : g.name)
            .Int("playtimeSeconds", g.playtime_seconds)
            .StringOrNull("lastPlayedAt", g.last_played_at)
            .StringOrNull("iconId", AssetId(snap, g.icon, hash))
            .StringOrNull("heroId", AssetId(snap, g.hero, hash))
            .StringOrNull("trophySetId", set != lib.set_for_title.end() ? set->second : "")
            .Bool("hidden", hidden::Contains(g.title_id))
            .EndObject();
    }
    w.EndArray().BeginArray("sets");
    for (const TrophySet &set : lib.trophies.sets) {
        w.BeginObject().String("id", set.id).String("version", set.version).EndObject();
    }
    w.EndArray().BeginArray("assets");
    for (const auto &[id, where] : snap.assets) w.String(id);
    w.EndArray().EndObject();
    snap.manifest = w.take();
    snap.fingerprint = Sha256::Hex(snap.manifest);
    return snap;
}

}
