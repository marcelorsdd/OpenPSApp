#include "routes/debug.hpp"

#include <algorithm>
#include <cstdlib>
#include <optional>

#include "formats/appdb.hpp"
#include "platform/platform.hpp"
#include "routes/routes.hpp"
#include "services/console_layout.hpp"
#include "services/playtime.hpp"
#include "services/tile.hpp"
#include "services/trophies.hpp"
#include "util/fs.hpp"
#include "util/json.hpp"
#include "util/log.hpp"
#include "util/stats.hpp"
#include "util/timeutil.hpp"

namespace playtrack::routes {
namespace {
constexpr size_t kMaxListed = 200;

void WriteFiles(JsonWriter &w, const std::string &console_dir) {
    w.BeginArray();
    for (const fs::Entry &e : fs::ListDir(platform::ConsolePath(console_dir))) {
        w.BeginObject().Key("name").String(e.name).Key("dir").Bool(e.is_dir).Key("size").Int(e.size).EndObject();
    }
    w.EndArray();
}

void WriteSets(JsonWriter &w, const std::string &console_dir) {
    w.BeginArray();
    size_t listed = 0;
    for (const fs::Entry &set : fs::ListDir(platform::ConsolePath(console_dir))) {
        if (!set.is_dir || listed++ >= kMaxListed) continue;
        w.BeginObject().Key("id").String(set.name).Key("files");
        WriteFiles(w, console_dir + "/" + set.name);
        w.EndObject();
    }
    w.EndArray();
}

void WriteAppDb(JsonWriter &w) {
    AppDb db;
    std::string err;
    w.Key("appDb").BeginObject().Key("path").String(layout::kAppDb);
    if (!db.Open(platform::ConsolePath(layout::kAppDb), &err)) {
        w.Key("ok").Bool(false).Key("error").String(err).EndObject();
        return;
    }
    w.Key("ok").Bool(true);

    std::vector<std::string> tables = db.Tables();
    w.Key("tables").BeginArray();
    for (const std::string &t : tables) w.String(t);
    w.EndArray();

    w.Key("details").BeginArray();
    for (const std::string &t : tables) {
        bool iconinfo = t.rfind(layout::kIconInfoPrefix, 0) == 0;
        if (!iconinfo && t != "tbl_contentinfo" && t != "tbl_appbrowse") continue;
        w.BeginObject().Key("name").String(t).Key("rows").Int(db.RowCount(t));
        w.Key("columns").BeginArray();
        for (const std::string &c : db.Columns(t)) w.String(c);
        w.EndArray();
        w.Key("sample").BeginArray();
        for (const auto &row : db.Sample(t, iconinfo ? 3 : 2)) {
            w.BeginObject();
            for (const auto &[col, value] : row) w.Key(col).String(value);
            w.EndObject();
        }
        w.EndArray().EndObject();
    }
    w.EndArray().EndObject();
}

void WriteTrophyDirs(JsonWriter &w, const TrophyReport &trophies) {
    w.Key("trophies").BeginObject().Key("dataDir").StringOrNull(trophies.data_dir);
    w.Key("conf").BeginObject().Key("path").String(layout::kTrophyConfDir).Key("sets");
    WriteSets(w, layout::kTrophyConfDir);
    w.EndObject();

    w.Key("homes").BeginArray();
    for (const fs::Entry &home : fs::ListDir(platform::ConsolePath(layout::kHomeDir))) {
        if (!home.is_dir) continue;
        std::string data_dir = layout::TrophyDataDir(home.name);
        w.BeginObject().Key("user").String(home.name).Key("dataPath").String(data_dir).Key("sets");
        WriteSets(w, data_dir);
        w.EndObject();
    }
    w.EndArray();

    w.Key("errors").BeginArray();
    for (const std::string &e : trophies.errors) w.String(e);
    w.EndArray().EndObject();
}

http::Response Sources(const Context &ctx) {
    std::optional<platform::User> user = platform::ForegroundUser();
    JsonWriter w;
    w.BeginObject().Key("platform").String(platform::Name()).Key("root").String(platform::Root());

    w.Key("user");
    if (user) {
        w.BeginObject().Key("id").Int(user->id).Key("idHex").String(user->id_hex).Key("name").String(user->name).EndObject();
    } else {
        w.Null();
    }

    WriteAppDb(w);

    PlaytimeReport playtime = CollectPlaytime(user);
    w.Key("playtime").BeginObject().Key("games").Int(static_cast<int64_t>(playtime.games.size()));
    w.Key("tables").BeginArray();
    for (const std::string &t : playtime.tables) w.String(t);
    w.EndArray().Key("error").StringOrNull(playtime.error).EndObject();

    WriteTrophyDirs(w, CollectTrophies(user, ctx.config.language));

    std::vector<fs::Entry> metas = fs::ListDir(platform::ConsolePath("/user/appmeta"));
    w.Key("appmeta").BeginObject().Key("count").Int(static_cast<int64_t>(metas.size())).Key("sample").BeginArray();
    for (size_t i = 0; i < metas.size() && i < 5; ++i) {
        w.BeginObject().Key("name").String(metas[i].name).Key("files");
        WriteFiles(w, layout::AppMetaDir(metas[i].name));
        w.EndObject();
    }
    w.EndArray().EndObject();

    w.EndObject();
    return http::Json(w.take());
}

const std::string *AllowedPathParam(const http::Request &req) {
    auto it = req.query.find("path");
    return it != req.query.end() && IsAllowedDebugPath(it->second) ? &it->second : nullptr;
}

http::Response List(const http::Request &req) {
    const std::string *path = AllowedPathParam(req);
    if (!path) return http::Error(403, "path not allowed");
    JsonWriter w;
    w.BeginObject().Key("path").String(*path).Key("entries");
    WriteFiles(w, *path);
    w.EndObject();
    return http::Json(w.take());
}

http::Response Download(const http::Request &req) {
    const std::string *path = AllowedPathParam(req);
    if (!path) return http::Error(403, "path not allowed");
    std::string local = platform::ConsolePath(*path);
    if (!fs::IsNonEmptyFile(local)) return http::Error(404, "not a file");
    return http::File(local, "application/octet-stream");
}

http::Response AppDbCopy(const std::string &data_dir) {
    AppDb db;
    std::string err;
    std::string dest = data_dir + "/app.db.copy";
    if (!db.Open(platform::ConsolePath(layout::kAppDb), &err) || !db.BackupTo(dest, &err)) {
        log::Error("app.db copy failed: %s", err.c_str());
        return http::Error(500, err);
    }
    return http::File(dest, "application/vnd.sqlite3");
}

http::Response Stats(const Context &ctx) {
    stats::Stats &s = stats::Get();
    JsonWriter w;
    w.BeginObject()
        .Key("version").String(ctx.version)
        .Key("platform").String(platform::Name())
        .Key("uptimeSec").Int(timeutil::NowUnix() - s.started_at)
        .Key("requests").Int(s.requests)
        .Key("collections").Int(s.collections)
        .Key("lastCollectMs").Int(s.last_collect_ms)
        .EndObject();
    return http::Json(w.take());
}

http::Response Tile() {
    tile::Status st = tile::GetStatus();
    JsonWriter w;
    w.BeginObject()
        .Key("titleId").String(tile::kTitleId)
        .Key("embedded").Bool(st.embedded)
        .Key("installed").Bool(st.installed)
        .Key("state").String(st.state)
        .Key("error").StringOrNull(st.error)
        .EndObject();
    return http::Json(w.take());
}

http::Response Log(const http::Request &req) {
    size_t lines = 200;
    if (auto it = req.query.find("lines"); it != req.query.end()) lines = std::strtoul(it->second.c_str(), nullptr, 10);
    http::Response res;
    res.content_type = "text/plain; charset=utf-8";
    for (const std::string &line : log::Tail(std::clamp<size_t>(lines, 1, 500))) res.body += line + "\n";
    return res;
}
}

bool IsAllowedDebugPath(const std::string &p) {
    if (p.empty() || p[0] != '/' || p.find("..") != std::string::npos) return false;
    static const char *const kPrefixes[] = {
        "/system_data/priv/mms", "/system_data/priv/appmeta", "/user/appmeta", "/user/trophy2",
    };
    for (const char *prefix : kPrefixes) {
        std::string pre = prefix;
        if (p == pre || p.rfind(pre + "/", 0) == 0) return true;
    }
    if (p.rfind("/user/app/", 0) == 0) {
        size_t sce = p.find("/sce_sys");
        return sce != std::string::npos && p.find('/', 10) == sce;
    }
    if (p == layout::kHomeDir) return true;
    if (p.rfind(std::string(layout::kHomeDir) + "/", 0) == 0) {
        std::string rest = p.substr(11);
        size_t slash = rest.find('/');
        if (slash == std::string::npos) return true;
        std::string tail = rest.substr(slash);
        return tail == "/trophy2" || tail.rfind("/trophy2/", 0) == 0;
    }
    return false;
}

void RegisterDebug(http::Router &router, const Context &ctx) {
    if (!ctx.config.debug) return;
    router.Get("/api/v1/debug/sources", [&ctx](const http::Request &) { return Sources(ctx); });
    router.Get("/api/v1/debug/stats", [&ctx](const http::Request &) { return Stats(ctx); });
    router.Get("/api/v1/debug/ls", List);
    router.Get("/api/v1/debug/file", Download);
    router.Get("/api/v1/debug/appdb", [&ctx](const http::Request &) { return AppDbCopy(ctx.data_dir); });
    router.Get("/api/v1/debug/log", Log);
    router.Get("/api/v1/debug/tile", [](const http::Request &) { return Tile(); });
}

}
