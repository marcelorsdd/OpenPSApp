#include <map>

#include "helpers.hpp"
#include "platform/platform.hpp"
#include "test.hpp"
#include "util/fs.hpp"
#include "util/sha256.hpp"

using namespace openpsapp;

namespace {
struct FileState {
    int64_t size = 0;
    int64_t mtime = 0;
    std::string sha;
    bool operator==(const FileState &o) const { return size == o.size && mtime == o.mtime && sha == o.sha; }
};

void Walk(const std::string &root, const std::string &rel, std::map<std::string, FileState> *out) {
    for (const fs::Entry &e : fs::ListDir(root + rel)) {
        std::string child = rel + "/" + e.name;
        if (child == "/data") continue;
        if (e.is_dir) {
            Walk(root, child, out);
            continue;
        }
        std::string data;
        fs::ReadFile(root + child, &data);
        auto st = fs::StatPath(root + child);
        (*out)[child] = {st ? st->size : -1, st ? st->mtime : -1, Sha256::Hex(data)};
    }
}
}

TEST(no_route_writes_console_data) {
    const std::string &root = platform::Root();
    std::map<std::string, FileState> before;
    Walk(root, "", &before);
    CHECK(before.count("/system_data/priv/mms/app.db") == 1);

    test::App app(true);
    fs::EnsureDir(app.ctx.data_dir);
    const char *const kPaths[] = {
        "/api/v1/health",
        "/api/v1/profile",
        "/api/v1/games",
        "/api/v1/games/PPSA01234",
        "/api/v1/trophy-sets",
        "/api/v1/trophy-sets/NPWR00001_00",
        "/api/v1/trophy-sets/NPWR00001_00/trophies",
        "/api/v1/debug/stats",
        "/api/v1/debug/sources",
        "/api/v1/debug/ls?path=/user/home",
        "/api/v1/debug/file?path=/system_data/priv/mms/app.db",
        "/api/v1/debug/appdb",
        "/api/v1/debug/log",
    };
    for (const char *target : kPaths) {
        http::Response res = test::Get(app.router, target);
        if (res.status != 200) std::printf("  %s -> %d %s\n", target, res.status, res.body.c_str());
        CHECK_EQ(res.status, 200);
    }

    std::map<std::string, FileState> after;
    Walk(root, "", &after);
    CHECK_EQ(after.size(), before.size());
    for (const auto &[path, state] : before) {
        auto it = after.find(path);
        bool same = it != after.end() && it->second == state;
        if (!same) std::printf("  changed: %s\n", path.c_str());
        CHECK(same);
    }
}
