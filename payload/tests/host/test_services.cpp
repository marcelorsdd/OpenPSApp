#include <unistd.h>

#include "fixture.hpp"
#include "platform/platform.hpp"
#include "services/hidden.hpp"
#include "services/icons.hpp"
#include "services/library.hpp"
#include "services/tile.hpp"
#include "test.hpp"
#include "util/fs.hpp"

using namespace playtrack;

TEST(trophy_set_lookup_prefers_app_db_path_and_falls_back_to_internal) {
    CHECK_EQ(TrophySetForTitle("PPSA01234", "/mnt/ext0/user/app/PPSA01234/sce_sys"), std::string(fixture::kSetId));
    CHECK_EQ(TrophySetForTitle("PPSA01234", ""), std::string(""));
    CHECK_EQ(TrophySetForTitle("PPSA05555", ""), std::string(fixture::kCorruptSetId));
    CHECK_EQ(TrophySetForTitle("PPSA05555", "/user/app/../../etc"), std::string(fixture::kCorruptSetId));
    CHECK_EQ(TrophySetForTitle("PPSA99999", ""), std::string(""));
}

TEST(icon_ids_follow_location_and_file_version) {
    icons::Location a {"/x/icon.png", 0, -1, "10-1"};
    icons::Location b = a;
    b.version = "10-2";
    std::string id = icons::Register(a);
    CHECK_EQ(id.size(), size_t(32));
    CHECK_EQ(icons::Register(a), id);
    CHECK(icons::Register(b) != id);
    CHECK(icons::Find(id).has_value());
    CHECK_EQ(icons::Url(a), "/api/v1/icons/" + id);
    CHECK_EQ(icons::Url({}), std::string(""));
}

TEST(hidden_games_persist_and_leave_the_totals) {
    std::string data_dir = platform::Root() + "/data/playtrack";
    fs::EnsureDir(data_dir);
    hidden::Load(data_dir);
    setenv("PLAYTRACK_USER", fixture::kUserHex, 1);
    auto lib = CurrentLibrary("pt-BR");

    ProfileTotals before = lib->Totals();
    CHECK_EQ(before.games, int64_t(3));
    CHECK_EQ(before.playtime_seconds, int64_t(10860));

    CHECK(hidden::Set("PPSA01234", true));
    ProfileTotals hidden_totals = lib->Totals();
    CHECK_EQ(hidden_totals.games, int64_t(2));
    CHECK_EQ(hidden_totals.playtime_seconds, int64_t(3660));
    CHECK_EQ(hidden_totals.trophies.total, before.trophies.total);

    hidden::Load(data_dir);
    CHECK(hidden::Contains("PPSA01234"));
    CHECK(hidden::Set("PPSA01234", false));
    CHECK(!hidden::Contains("PPSA01234"));
    unsetenv("PLAYTRACK_USER");
}

TEST(tile_counts_as_installed_once_its_appmeta_exists) {
    CHECK(!tile::GetStatus().installed);
    std::string appmeta = platform::Root() + "/user/appmeta/" + tile::kTitleId;
    CHECK(fs::EnsureDir(appmeta));
    CHECK(tile::GetStatus().installed);
    rmdir(appmeta.c_str());
}
