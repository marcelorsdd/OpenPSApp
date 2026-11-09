#include "services/trophies.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <unordered_map>

#include "cJSON.h"
#include "formats/npbind.hpp"
#include "formats/t2pd.hpp"
#include "formats/ucp.hpp"
#include "services/console_layout.hpp"
#include "util/fs.hpp"

namespace playtrack {
namespace {
Grade ParseGrade(const char *g) {
    if (!g) return Grade::Unknown;
    switch (g[0]) {
    case 'P': return Grade::Platinum;
    case 'G': return Grade::Gold;
    case 'S': return Grade::Silver;
    case 'B': return Grade::Bronze;
    default: return Grade::Unknown;
    }
}

const char *JsonString(const cJSON *obj, const char *key) {
    const cJSON *v = cJSON_GetObjectItemCaseSensitive(obj, key);
    return cJSON_IsString(v) ? v->valuestring : nullptr;
}

cJSON *ReadJson(const Ucp &ucp, const std::string &name) {
    const Ucp::Entry *entry = ucp.Find(name);
    std::string text;
    if (!entry || !ucp.Read(*entry, &text)) return nullptr;
    return cJSON_ParseWithLength(text.data(), text.size());
}

icons::Location IconAt(const Ucp &ucp, const std::string &entry_name, const std::string &version) {
    icons::Location where;
    if (const Ucp::Entry *e = ucp.Find(entry_name)) {
        where.path = ucp.path();
        where.offset = e->offset;
        where.length = static_cast<int64_t>(e->size);
        where.version = version;
    }
    return where;
}

std::string DataDir(const std::optional<platform::User> &user) {
    if (user) return layout::TrophyDataDir(user->id_hex);
    for (const fs::Entry &home : fs::ListDir(platform::ConsolePath(layout::kHomeDir))) {
        std::string dir = layout::TrophyDataDir(home.name);
        if (home.is_dir && !fs::ListDir(platform::ConsolePath(dir)).empty()) return dir;
    }
    return "";
}

struct CachedSet {
    std::string ucp_version;
    std::string dat_version;
    std::string language;
    TrophySet set;
};
std::mutex g_cache_mu;
std::map<std::string, CachedSet> g_cache;
}

const char *GradeName(Grade grade) {
    switch (grade) {
    case Grade::Platinum: return "platinum";
    case Grade::Gold: return "gold";
    case Grade::Silver: return "silver";
    case Grade::Bronze: return "bronze";
    default: return "unknown";
    }
}

int GradePoints(Grade grade) {
    switch (grade) {
    case Grade::Platinum: return 180;
    case Grade::Gold: return 90;
    case Grade::Silver: return 30;
    case Grade::Bronze: return 15;
    default: return 0;
    }
}

void Tally::Add(const Tally &other) {
    for (size_t i = 0; i < by_grade.size(); ++i) {
        by_grade[i].earned += other.by_grade[i].earned;
        by_grade[i].total += other.by_grade[i].total;
    }
    earned += other.earned;
    total += other.total;
}

Tally TrophySet::Count() const {
    Tally tally;
    for (const Trophy &t : trophies) {
        tally.total++;
        if (t.earned) tally.earned++;
        if (t.grade == Grade::Unknown) continue;
        GradeTally &g = tally.by_grade[static_cast<size_t>(t.grade)];
        g.total++;
        if (t.earned) g.earned++;
    }
    return tally;
}

int TrophySet::Progress() const {
    int total = 0, earned = 0;
    for (const Trophy &t : trophies) {
        if (t.grade == Grade::Platinum) continue;
        total += GradePoints(t.grade);
        if (t.earned) earned += GradePoints(t.grade);
    }
    return total ? earned * 100 / total : 0;
}

int64_t TrophySet::LastEarnedAt() const {
    int64_t last = 0;
    for (const Trophy &t : trophies) last = std::max(last, t.earned_at);
    return last;
}

std::vector<std::string> TropmetaLanguages(const Ucp &ucp) {
    static const std::string kPrefix = "tropmeta_";
    static const std::string kSuffix = ".json";
    std::vector<std::string> langs;
    for (const Ucp::Entry &e : ucp.entries()) {
        const std::string &n = e.name;
        if (n.size() <= kPrefix.size() + kSuffix.size() || n.rfind(kPrefix, 0) != 0 ||
            n.compare(n.size() - kSuffix.size(), kSuffix.size(), kSuffix) != 0) {
            continue;
        }
        langs.push_back(n.substr(kPrefix.size(), n.size() - kPrefix.size() - kSuffix.size()));
    }
    return langs;
}

std::vector<std::string> LanguageCandidates(const std::string &language, const std::vector<std::string> &available,
                                            const std::string &default_language) {
    auto primary = [](const std::string &tag) { return tag.substr(0, tag.find('-')); };
    auto has_script = [](const std::string &tag) {
        size_t dash = tag.find('-');
        return dash != std::string::npos && tag.size() - dash - 1 == 4;
    };

    std::vector<std::string> out;
    auto add = [&out](const std::string &tag) {
        if (!tag.empty() && std::find(out.begin(), out.end(), tag) == out.end()) out.push_back(tag);
    };
    add(language);
    if (!has_script(language)) {
        for (const std::string &tag : available) {
            if (primary(tag) == primary(language) && !has_script(tag)) add(tag);
        }
    }
    add(default_language);
    add("en-US");
    return out;
}

const TrophySet *TrophyReport::Find(const std::string &set_id) const {
    for (const TrophySet &s : sets) {
        if (s.id == set_id) return &s;
    }
    return nullptr;
}

bool BuildTrophySet(const std::string &id, const std::string &ucp_path, const std::string &dat_path,
                    const std::string &language, TrophySet *out, std::string *err) {
    Ucp ucp;
    if (!ucp.Open(ucp_path, err)) return false;
    std::string version = icons::FileVersion(ucp_path);

    cJSON *conf = ReadJson(ucp, "tropconf.json");
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(conf, "trophies");
    if (!cJSON_IsArray(list)) {
        cJSON_Delete(conf);
        *err = id + ": tropconf.json missing or without trophies";
        return false;
    }
    const char *default_lang = JsonString(conf, "defaultLanguage");

    std::vector<std::string> langs = LanguageCandidates(language, TropmetaLanguages(ucp), default_lang ? default_lang : "");

    TrophySet set;
    set.id = id;
    std::unordered_map<int, size_t> index;
    const cJSON *item = nullptr;
    cJSON_ArrayForEach(item, list) {
        const char *tid = JsonString(item, "id");
        if (!tid) continue;
        Trophy t;
        t.id = std::atoi(tid);
        t.grade = ParseGrade(JsonString(item, "grade"));
        t.hidden = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(item, "hidden"));
        char icon[32];
        std::snprintf(icon, sizeof(icon), "trop%04d.png", t.id);
        t.icon = IconAt(ucp, icon, version);
        index[t.id] = set.trophies.size();
        set.trophies.push_back(std::move(t));
    }
    cJSON_Delete(conf);

    cJSON *meta = nullptr;
    for (const std::string &lang : langs) {
        if ((meta = ReadJson(ucp, "tropmeta_" + lang + ".json"))) break;
    }
    for (size_t i = 0; !meta && i < ucp.entries().size(); ++i) {
        if (ucp.entries()[i].name.rfind("tropmeta_", 0) == 0) meta = ReadJson(ucp, ucp.entries()[i].name);
    }
    const cJSON *metadata = cJSON_GetObjectItemCaseSensitive(meta, "metadata");
    if (const char *name = JsonString(cJSON_GetObjectItemCaseSensitive(metadata, "titleMetadata"), "name")) {
        set.name = name;
    }
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(metadata, "trophyMetadata")) {
        const char *tid = JsonString(item, "id");
        auto it = tid ? index.find(std::atoi(tid)) : index.end();
        if (it == index.end()) continue;
        Trophy &t = set.trophies[it->second];
        if (const char *v = JsonString(item, "name")) t.name = v;
        if (const char *v = JsonString(item, "detail")) t.detail = v;
    }
    cJSON_Delete(meta);
    if (set.name.empty()) set.name = id;

    for (const std::string &lang : langs) {
        if (ucp.Find("icon0_" + lang + ".png")) {
            set.icon = IconAt(ucp, "icon0_" + lang + ".png", version);
            break;
        }
    }

    std::vector<TrophyState> states;
    if (!dat_path.empty() && !ReadTrophyStates(dat_path, &states, err)) return false;
    for (const TrophyState &s : states) {
        auto it = index.find(s.id);
        if (it == index.end()) continue;
        set.trophies[it->second].earned = s.earned;
        set.trophies[it->second].earned_at = s.earned_at;
    }

    *out = std::move(set);
    return true;
}

TrophyReport CollectTrophies(const std::optional<platform::User> &user, const std::string &language) {
    TrophyReport report;
    report.data_dir = DataDir(user);
    if (report.data_dir.empty()) return report;

    std::lock_guard<std::mutex> lock(g_cache_mu);
    for (const fs::Entry &entry : fs::ListDir(platform::ConsolePath(report.data_dir))) {
        if (!entry.is_dir || !IsNpCommId(entry.name)) continue;
        std::string ucp = platform::ConsolePath(std::string(layout::kTrophyConfDir) + "/" + entry.name + "/TROPHY.UCP");
        std::string dat = platform::ConsolePath(report.data_dir + "/" + entry.name + "/TRPTITLE.DAT");
        std::string ucp_version = icons::FileVersion(ucp);
        std::string dat_version = icons::FileVersion(dat);

        auto cached = g_cache.find(entry.name);
        if (cached != g_cache.end() && cached->second.ucp_version == ucp_version &&
            cached->second.dat_version == dat_version && cached->second.language == language) {
            report.sets.push_back(cached->second.set);
            continue;
        }
        TrophySet set;
        std::string err;
        if (!BuildTrophySet(entry.name, ucp, dat, language, &set, &err)) {
            report.errors.push_back(entry.name + ": " + err);
            continue;
        }
        g_cache[entry.name] = {ucp_version, dat_version, language, set};
        report.sets.push_back(std::move(set));
    }
    std::sort(report.sets.begin(), report.sets.end(), [](const TrophySet &a, const TrophySet &b) {
        if (a.LastEarnedAt() != b.LastEarnedAt()) return a.LastEarnedAt() > b.LastEarnedAt();
        return a.id < b.id;
    });
    return report;
}

}
