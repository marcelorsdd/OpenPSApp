#include "services/playtime.hpp"

#include <strings.h>

#include <algorithm>
#include <cstdio>
#include <initializer_list>
#include <set>
#include <unordered_map>

#include "formats/appdb.hpp"
#include "services/console_layout.hpp"
#include "util/fs.hpp"
#include "util/timeutil.hpp"

namespace playtrack {
namespace {
struct TitleMeta {
    std::string name;
    std::string metadata_dir;
    bool is_app = false;
};

std::string ColumnOr(const std::set<std::string> &cols, const char *col, const char *fallback) {
    return cols.count(col) ? std::string("COALESCE(") + col + "," + fallback + ")" : fallback;
}

std::set<std::string> ColumnSet(const AppDb &db, const std::string &table) {
    std::vector<std::string> cols = db.Columns(table);
    return {cols.begin(), cols.end()};
}

std::unordered_map<std::string, TitleMeta> LoadTitleMeta(const AppDb &db) {
    std::unordered_map<std::string, TitleMeta> meta;

    std::set<std::string> cols = ColumnSet(db, "tbl_contentinfo");
    if (!cols.empty()) {
        std::string sql = "SELECT titleId, " + ColumnOr(cols, "titleName", "''") + ", " +
                          ColumnOr(cols, "is3rdPartyVideoApp", "0") + ", " + ColumnOr(cols, "categoryType", "0") + ", " +
                          ColumnOr(cols, "metaDataPath", "''") + " FROM tbl_contentinfo";
        db.ForEachRow(sql, {}, [&meta](const AppDb::Row &r) {
            TitleMeta &m = meta[r.Text(0)];
            if (m.name.empty()) m.name = r.Text(1);
            if (m.metadata_dir.empty()) m.metadata_dir = r.Text(4);

            m.is_app = m.is_app || r.Int(2) != 0 || (r.Int(3) & 0x10000) != 0;
        });
    }

    if (ColumnSet(db, "tbl_appbrowse").count("titleName")) {
        db.ForEachRow("SELECT titleId, titleName FROM tbl_appbrowse WHERE titleName IS NOT NULL AND titleName != ''", {},
                      [&meta](const AppDb::Row &r) {
                          TitleMeta &m = meta[r.Text(0)];
                          if (m.name.empty()) m.name = r.Text(1);
                      });
    }
    return meta;
}

std::vector<std::string> PickTables(const AppDb &db, const std::optional<platform::User> &user) {
    std::vector<std::string> tables;
    for (const std::string &t : db.Tables(std::string(layout::kIconInfoPrefix) + "%")) {
        if (ColumnSet(db, t).count("playedTimeOnConsole")) tables.push_back(t);
    }
    if (user) {
        char decimal[16];
        std::snprintf(decimal, sizeof(decimal), "%010u", static_cast<uint32_t>(user->id));
        for (const std::string &suffix : {std::string(decimal), user->id_hex}) {
            std::string mine = layout::kIconInfoPrefix + suffix;
            for (const std::string &t : tables) {
                if (strcasecmp(t.c_str(), mine.c_str()) == 0) return {t};
            }
        }
    }
    return tables;
}

icons::Location FindTitleImage(const std::string &title_id, std::initializer_list<const char *> files) {
    static const char *const kDirs[] = {
        "/user/appmeta/%s/",
        "/user/appmeta/external/%s/",
        "/system_data/priv/appmeta/%s/",
        "/system_data/priv/appmeta/external/%s/",
        "/user/app/%s/sce_sys/",
    };
    char buf[256];
    for (const char *file : files) {
        for (const char *pattern : kDirs) {
            std::snprintf(buf, sizeof(buf), pattern, title_id.c_str());
            std::string path = platform::ConsolePath(buf + std::string(file));
            if (fs::IsNonEmptyFile(path)) return {path, 0, -1, icons::FileVersion(path)};
        }
    }
    return {};
}
}

const GamePlaytime *PlaytimeReport::Find(const std::string &title_id) const {
    for (const GamePlaytime &g : games) {
        if (g.title_id == title_id) return &g;
    }
    return nullptr;
}

bool IsGameTitleId(std::string_view title_id) {
    static constexpr std::string_view kPrefixes[] = {"PPSA", "CUSA"};
    for (std::string_view p : kPrefixes) {
        if (title_id.substr(0, p.size()) == p) return true;
    }
    return false;
}

PlaytimeReport CollectPlaytime(const std::optional<platform::User> &user) {
    PlaytimeReport report;
    AppDb db;
    if (!db.Open(platform::ConsolePath(layout::kAppDb), &report.error)) return report;

    report.tables = PickTables(db, user);
    if (report.tables.empty()) {
        report.error = "no tbl_iconinfo_* table with playedTimeOnConsole";
        return report;
    }

    std::unordered_map<std::string, TitleMeta> meta = LoadTitleMeta(db);
    std::unordered_map<std::string, GamePlaytime> by_title;

    for (const std::string &table : report.tables) {
        std::set<std::string> cols = ColumnSet(db, table);
        std::string last = cols.count("lastPlayedDateOnConsole") ? ColumnOr(cols, "lastPlayedDateOnConsole", "''")
                                                                  : ColumnOr(cols, "lastAccessTime", "''");
        std::string sql = "SELECT titleId, COALESCE(playedTimeOnConsole,0), " + last + " FROM " + table;
        db.ForEachRow(sql, {}, [&](const AppDb::Row &r) {
            std::string title_id = r.Text(0);
            if (!IsGameTitleId(title_id)) return;
            auto m = meta.find(title_id);
            if (m != meta.end() && m->second.is_app) return;

            int64_t played = r.Int(1);
            std::string last_played = timeutil::NormalizeTimestamp(r.Text(2));
            if (played <= 0 && last_played.empty()) return;

            GamePlaytime &g = by_title[title_id];
            g.title_id = title_id;
            g.playtime_seconds = std::max(g.playtime_seconds, played);
            if (last_played > g.last_played_at) g.last_played_at = last_played;
        });
    }

    for (auto &[title_id, game] : by_title) {
        auto m = meta.find(title_id);
        game.name = m != meta.end() && !m->second.name.empty() ? m->second.name : title_id;
        if (m != meta.end()) game.metadata_dir = m->second.metadata_dir;
        game.icon = FindTitleImage(title_id, {"icon0.png"});
        game.hero = FindTitleImage(title_id, {"pic0.png", "pic1.png"});
        report.games.push_back(std::move(game));
    }
    std::sort(report.games.begin(), report.games.end(), [](const GamePlaytime &a, const GamePlaytime &b) {
        if (a.last_played_at != b.last_played_at) return a.last_played_at > b.last_played_at;
        return a.title_id < b.title_id;
    });
    return report;
}

}
