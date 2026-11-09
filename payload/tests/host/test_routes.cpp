#include <cstdlib>

#include "fixture.hpp"
#include "helpers.hpp"
#include "routes/debug.hpp"
#include "services/hidden.hpp"
#include "test.hpp"
#include "util/fs.hpp"

using namespace playtrack;

namespace {
std::string FirstIconUrl(const std::string &body) {
    size_t at = body.find("\"iconUrl\":\"");
    if (at == std::string::npos) return "";
    at += 11;
    return body.substr(at, body.find('"', at) - at);
}
}

TEST(web_serves_the_embedded_page_gzipped) {
    test::App app;
    http::Response page = test::Get(app.router, "/");
    CHECK_EQ(page.status, 200);
    CHECK_EQ(page.content_type, std::string("text/html; charset=utf-8"));
    CHECK_EQ(page.content_encoding, std::string("gzip"));
    CHECK(page.body.size() > 2 && page.body.compare(0, 2, "\x1f\x8b") == 0);
    CHECK_EQ(page.cache_control, std::string("no-cache"));
    CHECK_EQ(test::Get(app.router, "/assets/missing.js").status, 404);
    CHECK_EQ(test::Get(app.router, "/api/v1/nope").status, 404);
}

TEST(health_reports_the_console_language) {
    test::App app;
    app.ctx.language = "es-419";
    CHECK(test::Get(app.router, "/api/v1/health").body.find("\"language\":\"es-419\"") != std::string::npos);
}

TEST(debug_routes_are_off_unless_enabled) {
    test::App off;
    CHECK_EQ(test::Get(off.router, "/api/v1/debug/stats").status, 404);
    CHECK_EQ(test::Get(off.router, "/api/v1/debug/file?path=/system_data/priv/mms/app.db").status, 404);
    CHECK_EQ(test::Get(off.router, "/api/v1/health").status, 200);

    test::App on(true);
    CHECK_EQ(test::Get(on.router, "/api/v1/debug/stats").status, 200);
    CHECK_EQ(test::Get(on.router, "/api/v1/debug/file?path=/etc/passwd").status, 403);
}

TEST(debug_paths_are_allowlisted) {
    CHECK(routes::IsAllowedDebugPath("/system_data/priv/mms/app.db"));
    CHECK(routes::IsAllowedDebugPath("/user/trophy2/nobackup/conf/NPWR00001_00/TROPHY.UCP"));
    CHECK(routes::IsAllowedDebugPath("/user/home"));
    CHECK(routes::IsAllowedDebugPath("/user/home/1eb71bbd"));
    CHECK(routes::IsAllowedDebugPath("/user/home/1eb71bbd/trophy2/nobackup/data/NPWR00001_00/TRPTITLE.DAT"));
    CHECK(routes::IsAllowedDebugPath("/user/appmeta/PPSA01234/icon0.png"));
    CHECK(routes::IsAllowedDebugPath("/user/app/PPSA03671/sce_sys/trophy2/npbind.dat"));

    CHECK(!routes::IsAllowedDebugPath("/user/home/1eb71bbd/savedata/x"));
    CHECK(!routes::IsAllowedDebugPath("/user/trophy2/../home/x"));
    CHECK(!routes::IsAllowedDebugPath("/data/playtrack/config.json"));
    CHECK(!routes::IsAllowedDebugPath("/system_data/priv/mmsx"));
    CHECK(!routes::IsAllowedDebugPath("/user/app/PPSA03671/eboot.bin"));
    CHECK(!routes::IsAllowedDebugPath("/user/app/PPSA03671/data/sce_sys/x"));
    CHECK(!routes::IsAllowedDebugPath("relative/path"));
    CHECK(!routes::IsAllowedDebugPath("/etc/passwd"));
}

TEST(routes_link_games_and_sets_and_serve_trophy_icons) {
    setenv("PLAYTRACK_USER", fixture::kUserHex, 1);
    test::App app;

    CHECK(test::Get(app.router, "/api/v1/games/PPSA01234").body.find("\"trophySetId\":\"NPWR00001_00\"") !=
          std::string::npos);

    std::string set = test::Get(app.router, "/api/v1/trophy-sets/NPWR00001_00").body;
    CHECK(set.find("\"titleId\":\"PPSA01234\"") != std::string::npos);
    CHECK(set.find("\"progress\":77") != std::string::npos);
    CHECK(set.find("\"gold\":{\"earned\":1,\"total\":1}") != std::string::npos);

    std::string trophies = test::Get(app.router, "/api/v1/trophy-sets/NPWR00001_00/trophies").body;
    CHECK(trophies.find("\"earnedAt\":\"2025-11-07T20:00:00Z\"") != std::string::npos);
    std::string icon_url = FirstIconUrl(trophies);
    CHECK(!icon_url.empty());

    http::Response icon = test::Get(app.router, icon_url);
    CHECK_EQ(icon.status, 200);
    std::string bytes;
    fs::ReadRange(icon.file_path, icon.file_offset, icon.file_length, &bytes);
    CHECK_EQ(bytes, std::string("\x89PNG\r\n\x1a\ntrop0000", 16));

    CHECK_EQ(test::Get(app.router, "/api/v1/trophy-sets/NPWR99999_00").status, 404);
    CHECK_EQ(test::Get(app.router, "/api/v1/icons/00000000000000000000000000000000").status, 404);
    CHECK(test::Get(app.router, "/api/v1/profile").body.find("\"earned\":2,\"total\":4") != std::string::npos);
    unsetenv("PLAYTRACK_USER");
}

TEST(hide_and_unhide_routes_update_the_game_and_profile) {
    hidden::Load(platform::Root() + "/data/playtrack");
    setenv("PLAYTRACK_USER", fixture::kUserHex, 1);
    test::App app;

    CHECK(test::Get(app.router, "/api/v1/profile").body.find("\"games\":3,\"playtimeSeconds\":10860") !=
          std::string::npos);
    http::Response hid = test::Call(app.router, "POST", "/api/v1/games/PPSA01234/hide");
    CHECK_EQ(hid.status, 200);
    CHECK(hid.body.find("\"hidden\":true") != std::string::npos);
    CHECK(test::Get(app.router, "/api/v1/profile").body.find("\"games\":2,\"playtimeSeconds\":3660") !=
          std::string::npos);

    CHECK_EQ(test::Call(app.router, "POST", "/api/v1/games/PPSA01234/unhide").status, 200);
    CHECK(test::Get(app.router, "/api/v1/games/PPSA01234").body.find("\"hidden\":false") != std::string::npos);
    CHECK_EQ(test::Call(app.router, "POST", "/api/v1/games/PPSA00000/hide").status, 404);
    unsetenv("PLAYTRACK_USER");
}
