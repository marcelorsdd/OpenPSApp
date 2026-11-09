#include "services/library.hpp"

#include <algorithm>
#include <mutex>

#include "formats/npbind.hpp"
#include "services/console_layout.hpp"
#include "services/hidden.hpp"
#include "util/fs.hpp"
#include "util/log.hpp"
#include "util/stats.hpp"
#include "util/timeutil.hpp"

namespace playtrack {
namespace {
void RegisterIcons(const Library &lib) {
    for (const GamePlaytime &g : lib.playtime.games) {
        icons::Register(g.icon);
        icons::Register(g.hero);
    }
    for (const TrophySet &set : lib.trophies.sets) {
        icons::Register(set.icon);
        for (const Trophy &t : set.trophies) icons::Register(t.icon);
    }
}

std::shared_ptr<const Library> Build(const std::optional<platform::User> &user, const std::string &language) {
    int64_t start = timeutil::NowMillis();
    auto lib = std::make_shared<Library>();
    lib->user = user;
    lib->playtime = CollectPlaytime(lib->user);
    lib->trophies = CollectTrophies(lib->user, language);

    for (const GamePlaytime &g : lib->playtime.games) {
        std::string set = TrophySetForTitle(g.title_id, g.metadata_dir);
        if (set.empty()) continue;
        lib->set_for_title[g.title_id] = set;
        lib->title_for_set.emplace(set, g.title_id);
    }
    RegisterIcons(*lib);

    for (const std::string &e : lib->trophies.errors) log::Error("trophies: %s", e.c_str());
    stats::Get().collections++;
    stats::Get().last_collect_ms = timeutil::NowMillis() - start;
    return lib;
}
}

ProfileTotals Library::Totals() const {
    ProfileTotals totals;
    for (const GamePlaytime &g : playtime.games) {
        if (hidden::Contains(g.title_id)) continue;
        totals.games++;
        totals.playtime_seconds += g.playtime_seconds;
    }
    for (const TrophySet &set : trophies.sets) totals.trophies.Add(set.Count());
    return totals;
}

std::vector<RecentTrophy> Library::RecentTrophies(size_t limit) const {
    std::vector<RecentTrophy> out;
    for (const TrophySet &set : trophies.sets) {
        for (const Trophy &t : set.trophies) {
            if (t.earned) out.push_back({&set, &t});
        }
    }
    std::sort(out.begin(), out.end(), [](const RecentTrophy &a, const RecentTrophy &b) {
        if (a.trophy->earned_at != b.trophy->earned_at) return a.trophy->earned_at > b.trophy->earned_at;
        if (a.set->id != b.set->id) return a.set->id < b.set->id;
        return a.trophy->id < b.trophy->id;
    });
    if (out.size() > limit) out.resize(limit);
    return out;
}

std::string TrophySetForTitle(const std::string &title_id, const std::string &metadata_dir) {
    std::vector<std::string> dirs;
    if (!metadata_dir.empty() && metadata_dir[0] == '/' && metadata_dir.find("..") == std::string::npos) {
        dirs.push_back(metadata_dir);
    }
    dirs.push_back(layout::InternalSceSys(title_id));

    for (const std::string &dir : dirs) {
        std::string data;
        if (!fs::ReadFile(platform::ConsolePath(dir + "/trophy2/npbind.dat"), &data, 64 * 1024)) continue;
        std::string id = ParseNpCommId(data);
        if (!id.empty()) return id;
    }
    return "";
}

std::shared_ptr<const Library> CurrentLibrary(const std::string &language) {
    static std::mutex mu;
    static std::shared_ptr<const Library> cached;
    static int64_t built_at = 0;

    std::optional<platform::User> user = platform::ForegroundUser();
    std::lock_guard<std::mutex> lock(mu);
    int64_t now = timeutil::NowUnix();
    bool same_user = cached && cached->user.has_value() == user.has_value() && (!user || cached->user->id == user->id);
    if (!same_user || now - built_at >= kLibraryTtlSec) {
        cached = Build(user, language);
        built_at = now;
    }
    return cached;
}

}
