#include "routes/debug.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <optional>

#include "platform/platform.hpp"
#include "routes/routes.hpp"
#include "services/console_layout.hpp"
#include "services/diagnostics.hpp"
#include "services/playtime.hpp"
#include "services/tile.hpp"
#include "services/trophies.hpp"
#include "util/fs.hpp"
#include "util/json.hpp"
#include "util/log.hpp"
#include "util/stats.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::routes {
namespace {
constexpr size_t kMaxListed = 200;

void WriteFiles(JsonWriter &w, const std::string &console_dir) {
    w.BeginArray();
    for (const fs::Entry &e : fs::ListDir(platform::ConsolePath(console_dir))) {
        w.BeginObject().String("name", e.name).Bool("dir", e.is_dir).Int("size", e.size).EndObject();
    }
    w.EndArray();
}

void WriteSets(JsonWriter &w, const std::string &console_dir) {
    w.BeginArray();
    size_t listed = 0;
    for (const fs::Entry &set : fs::ListDir(platform::ConsolePath(console_dir))) {
        if (!set.is_dir || listed++ >= kMaxListed) continue;
        w.BeginObject().String("id", set.name).Key("files");
        WriteFiles(w, console_dir + "/" + set.name);
        w.EndObject();
    }
    w.EndArray();
}

void WriteAppDb(JsonWriter &w) {
    diagnostics::AppDbReport db = diagnostics::InspectAppDb();
    w.BeginObject("appDb").String("path", layout::kAppDb);
    if (!db.ok) {
        w.Bool("ok", false).String("error", db.error).EndObject();
        return;
    }
    w.Bool("ok", true);

    w.BeginArray("tables");
    for (const std::string &t : db.tables) w.String(t);
    w.EndArray();

    w.BeginArray("details");
    for (const diagnostics::AppDbTable &t : db.details) {
        w.BeginObject().String("name", t.name).Int("rows", t.rows);
        w.BeginArray("columns");
        for (const std::string &c : t.columns) w.String(c);
        w.EndArray();
        w.BeginArray("sample");
        for (const auto &row : t.sample) {
            w.BeginObject();
            for (const auto &[col, value] : row) w.String(col, value);
            w.EndObject();
        }
        w.EndArray().EndObject();
    }
    w.EndArray().EndObject();
}

void WriteTrophyDirs(JsonWriter &w, const TrophyReport &trophies) {
    w.BeginObject("trophies").StringOrNull("dataDir", trophies.data_dir);
    w.BeginObject("conf").String("path", layout::kTrophyConfDir).Key("sets");
    WriteSets(w, layout::kTrophyConfDir);
    w.EndObject();

    w.BeginArray("homes");
    for (const fs::Entry &home : fs::ListDir(platform::ConsolePath(layout::kHomeDir))) {
        if (!home.is_dir) continue;
        std::string data_dir = layout::TrophyDataDir(home.name);
        w.BeginObject().String("user", home.name).String("dataPath", data_dir).Key("sets");
        WriteSets(w, data_dir);
        w.EndObject();
    }
    w.EndArray();

    w.BeginArray("errors");
    for (const std::string &e : trophies.errors) w.String(e);
    w.EndArray().EndObject();
}

http::Response Sources(const Context &ctx) {
    std::optional<platform::User> user = platform::ForegroundUser();
    JsonWriter w;
    w.BeginObject().String("platform", platform::Name()).String("root", platform::Root());

    w.Key("user");
    if (user) {
        w.BeginObject().Int("id", user->id).String("idHex", user->id_hex).String("name", user->name).EndObject();
    } else {
        w.Null();
    }

    WriteAppDb(w);

    PlaytimeReport playtime = CollectPlaytime(user);
    w.BeginObject("playtime").Int("games", static_cast<int64_t>(playtime.games.size()));
    w.BeginArray("tables");
    for (const std::string &t : playtime.tables) w.String(t);
    w.EndArray().StringOrNull("error", playtime.error).EndObject();

    WriteTrophyDirs(w, CollectTrophies(user, ctx.language));

    std::vector<fs::Entry> metas = fs::ListDir(platform::ConsolePath("/user/appmeta"));
    w.BeginObject("appmeta").Int("count", static_cast<int64_t>(metas.size())).BeginArray("sample");
    for (size_t i = 0; i < metas.size() && i < 5; ++i) {
        w.BeginObject().String("name", metas[i].name).Key("files");
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
    if (!path) return http::Error(403, "path_not_allowed");
    JsonWriter w;
    w.BeginObject().String("path", *path).Key("entries");
    WriteFiles(w, *path);
    w.EndObject();
    return http::Json(w.take());
}

http::Response Download(const http::Request &req) {
    const std::string *path = AllowedPathParam(req);
    if (!path) return http::Error(403, "path_not_allowed");
    std::string local = platform::ConsolePath(*path);
    if (!fs::IsNonEmptyFile(local)) return http::Error(404, "not_found");
    return http::File(local, "application/octet-stream");
}

http::Response AppDbCopy(const std::string &data_dir) {
    std::string err;
    std::string dest = data_dir + "/app.db.copy";
    if (!diagnostics::CopyAppDb(dest, &err)) {
        log::Error("app.db copy failed: %s", err.c_str());
        return http::Error(500, "appdb_copy_failed");
    }
    return http::File(dest, "application/vnd.sqlite3");
}

http::Response Stats(const Context &ctx) {
    stats::Stats &s = stats::Get();
    JsonWriter w;
    w.BeginObject()
        .String("version", ctx.version)
        .String("platform", platform::Name())
        .Int("uptimeSec", timeutil::NowUnix() - s.started_at)
        .Int("requests", s.requests)
        .Int("collections", s.collections)
        .Int("lastCollectMs", s.last_collect_ms)
        .EndObject();
    return http::Json(w.take());
}

http::Response Tile() {
    tile::Status st = tile::GetStatus();
    JsonWriter w;
    w.BeginObject()
        .String("titleId", tile::kTitleId)
        .Bool("embedded", st.embedded)
        .Bool("installed", st.installed)
        .String("state", st.state)
        .StringOrNull("error", st.error)
        .EndObject();
    return http::Json(w.take());
}

http::Response Identity(const Context &ctx) {
    std::optional<platform::User> user = platform::ForegroundUser();
    std::string account = user ? platform::AccountIdHex(*user) : "";
    std::string device = platform::DeviceIdHex();
    TrophyReport trophies = CollectTrophies(user, ctx.language);

    JsonWriter w;
    w.BeginObject().Key("user");
    if (user) {
        w.BeginObject().String("idHex", user->id_hex).String("name", user->name).EndObject();
    } else {
        w.Null();
    }
    w.StringOrNull("accountId", account).StringOrNull("deviceId", device).BeginArray("trophyFiles");
    for (const diagnostics::TrophyFileOwner &file : diagnostics::TrophyFileOwners(trophies)) {
        char uid[16];
        std::snprintf(uid, sizeof(uid), "%08x", file.user_id);
        w.BeginObject().String("set", file.set_id).Bool("ok", file.ok).StringOrNull("error", file.error);
        if (file.ok) {
            w.String("fileUserId", uid)
                .String("fileAccountId", file.account_id_hex)
                .String("fileDeviceId", file.device_id_hex)
                .Bool("userIdMatches", user && uid == user->id_hex)
                .Bool("accountIdMatches", !account.empty() && file.account_id_hex == account)
                .Bool("deviceIdMatches", !device.empty() && file.device_id_hex == device);
        }
        w.EndObject();
    }
    w.EndArray().EndObject();
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
        "/system_data/priv/mms",
        "/system_data/priv/appmeta",
        "/user/appmeta",
        "/user/trophy2",
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
    router.Get("/api/v1/debug/identity", [&ctx](const http::Request &) { return Identity(ctx); });
}

}
