#include "fixture.hpp"
#include "services/playtime.hpp"
#include "test.hpp"

using namespace playtrack;

TEST(playtime_reads_only_the_foreground_profile) {
    PlaytimeReport r = CollectPlaytime(fixture::User());
    CHECK_EQ(r.error, std::string(""));
    CHECK_EQ(r.tables.size(), size_t(1));
    CHECK_EQ(r.tables.empty() ? std::string() : r.tables[0], std::string("tbl_iconinfo_0515316669"));
    CHECK_EQ(r.games.size(), size_t(3));
    CHECK(r.Find("PPSA07777") == nullptr);

    const GamePlaytime *astro = r.Find("PPSA01234");
    CHECK(astro != nullptr);
    if (astro) {
        CHECK_EQ(astro->name, std::string("Astro Test"));
        CHECK_EQ(astro->playtime_seconds, int64_t(7200));
        CHECK_EQ(astro->last_played_at, std::string("2026-11-07T20:15:00Z"));
        CHECK(astro->icon.path.find("/user/appmeta/PPSA01234/icon0.png") != std::string::npos);
        CHECK(!astro->icon.version.empty());
        CHECK(astro->hero.path.find("/user/appmeta/PPSA01234/pic0.png") != std::string::npos);
        CHECK_EQ(astro->metadata_dir, std::string("/mnt/ext0/user/app/PPSA01234/sce_sys"));
    }
}

TEST(playtime_orders_by_last_played_and_skips_apps) {
    PlaytimeReport r = CollectPlaytime(fixture::User());
    CHECK(r.games.size() == 3);
    if (r.games.size() == 3) {
        CHECK_EQ(r.games[0].title_id, std::string("PPSA01234"));
        CHECK_EQ(r.games[1].title_id, std::string("CUSA00001"));
        CHECK_EQ(r.games[2].title_id, std::string("PPSA05555"));
    }
    CHECK(r.Find("PPSA01650") == nullptr);
    CHECK(r.Find("PMGR09999") == nullptr);
    CHECK(r.Find("PPSA09999") == nullptr);
}

TEST(playtime_falls_back_to_appbrowse_name_and_external_icon) {
    PlaytimeReport r = CollectPlaytime(fixture::User());
    const GamePlaytime *browse = r.Find("PPSA05555");
    CHECK(browse != nullptr);
    if (browse) {
        CHECK_EQ(browse->name, std::string("Browse Name"));
        CHECK_EQ(browse->last_played_at, std::string(""));
        CHECK(browse->icon.path.empty());
        CHECK(browse->hero.path.empty());
    }
    const GamePlaytime *old = r.Find("CUSA00001");
    CHECK(old != nullptr);
    if (old) CHECK(old->icon.path.find("/user/appmeta/external/CUSA00001/icon0.png") != std::string::npos);
}

TEST(playtime_without_user_merges_profiles_by_max) {
    PlaytimeReport r = CollectPlaytime(std::nullopt);
    CHECK_EQ(r.tables.size(), size_t(2));
    const GamePlaytime *astro = r.Find("PPSA01234");
    CHECK(astro != nullptr);
    if (astro) {
        CHECK_EQ(astro->playtime_seconds, int64_t(99999));
        CHECK_EQ(astro->last_played_at, std::string("2026-11-08T07:00:00Z"));
    }
    CHECK(r.Find("PPSA07777") != nullptr);
}

TEST(playtime_unknown_profile_falls_back_to_merge) {
    platform::User nobody = fixture::User();
    nobody.id = 0x0badbeef;
    nobody.id_hex = "0badbeef";
    CHECK_EQ(CollectPlaytime(nobody).tables.size(), size_t(2));
}

TEST(game_title_ids) {
    CHECK(IsGameTitleId("PPSA01234"));
    CHECK(IsGameTitleId("CUSA00001"));
    CHECK(!IsGameTitleId("PMGR09999"));
    CHECK(!IsGameTitleId("NPXS40000"));
    CHECK(!IsGameTitleId(""));
}
