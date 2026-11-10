#include "fixture.hpp"
#include "formats/ucp.hpp"
#include "services/trophies.hpp"
#include "test.hpp"

using namespace openpsapp;

TEST(trophies_join_catalog_progress_and_language) {
    TrophyReport r = CollectTrophies(fixture::User(), "pt-BR");
    CHECK_EQ(r.data_dir, std::string("/user/home/1eb71bbd/trophy2/nobackup/data"));
    const TrophySet *set = r.Find(fixture::kSetId);
    CHECK(set != nullptr);
    if (!set) return;

    CHECK_EQ(set->name, std::string("Astro Teste"));
    CHECK_EQ(set->trophies.size(), size_t(4));
    if (set->trophies.size() != 4) return;
    const Trophy &plat = set->trophies[0], &gold = set->trophies[1], &silver = set->trophies[2],
                 &bronze = set->trophies[3];

    CHECK_EQ(plat.name, std::string("Corrida Platina"));
    CHECK_EQ(std::string(GradeName(plat.grade)), std::string("platinum"));
    CHECK(!plat.earned);
    CHECK(gold.earned);
    CHECK_EQ(gold.earned_at, int64_t(1762545600));
    CHECK(silver.hidden);
    CHECK(!silver.earned);
    CHECK(bronze.earned);
    CHECK_EQ(bronze.detail, std::string("Primeiros Passos detail"));

    CHECK_EQ(set->Progress(), 77);
    CHECK_EQ(set->LastEarnedAt(), int64_t(1762545600));
    CHECK(!set->icon.path.empty());
    CHECK(!gold.icon.path.empty());
    CHECK_EQ(gold.icon.version, set->icon.version);
}

TEST(trophy_tally_counts_per_grade_and_overall) {
    TrophyReport r = CollectTrophies(fixture::User(), "pt-BR");
    const TrophySet *set = r.Find(fixture::kSetId);
    CHECK(set != nullptr);
    if (!set) return;
    Tally t = set->Count();
    CHECK_EQ(t.earned, 2);
    CHECK_EQ(t.total, 4);
    CHECK_EQ(t.by_grade[0].total, 1);
    CHECK_EQ(t.by_grade[1].earned, 1);
    CHECK_EQ(t.by_grade[3].earned, 1);

    Tally sum;
    sum.Add(t);
    sum.Add(t);
    CHECK_EQ(sum.earned, 4);
    CHECK_EQ(sum.by_grade[1].total, 2);
}

TEST(trophies_fall_back_to_default_language) {
    TrophyReport r = CollectTrophies(fixture::User(), "xx-XX");
    const TrophySet *set = r.Find(fixture::kSetId);
    CHECK(set != nullptr);
    if (set) CHECK_EQ(set->name, std::string("Astro Test"));
}

TEST(corrupt_trophy_file_is_reported_without_hiding_other_sets) {
    TrophyReport r = CollectTrophies(fixture::User(), "pt-BR");
    CHECK(r.Find(fixture::kSetId) != nullptr);
    CHECK(r.Find(fixture::kCorruptSetId) == nullptr);
    CHECK_EQ(r.errors.size(), size_t(1));
    if (!r.errors.empty()) CHECK(r.errors[0].find(fixture::kCorruptSetId) != std::string::npos);
}

TEST(trophies_use_a_sibling_language_before_the_default) {
    TrophyReport pt_report = CollectTrophies(fixture::User(), "pt-PT");
    const TrophySet *pt = pt_report.Find(fixture::kSetId);
    CHECK(pt != nullptr);
    if (pt) CHECK_EQ(pt->name, std::string("Astro Teste"));

    TrophyReport es_report = CollectTrophies(fixture::User(), "es-419");
    const TrophySet *es = es_report.Find(fixture::kSetId);
    CHECK(es != nullptr);
    if (es) CHECK_EQ(es->name, std::string("Astro Test"));
}

TEST(language_candidates_skip_other_scripts_and_keep_fallbacks) {
    std::vector<std::string> pt = LanguageCandidates("pt-PT", {"en-US", "pt-BR"}, "en-US");
    CHECK_EQ(pt.size(), size_t(3));
    if (pt.size() == 3) {
        CHECK_EQ(pt[0], std::string("pt-PT"));
        CHECK_EQ(pt[1], std::string("pt-BR"));
        CHECK_EQ(pt[2], std::string("en-US"));
    }
    std::vector<std::string> zh = LanguageCandidates("zh-Hant", {"zh-Hans", "ja-JP"}, "ja-JP");
    CHECK_EQ(zh.size(), size_t(3));
    if (zh.size() == 3) {
        CHECK_EQ(zh[1], std::string("ja-JP"));
        CHECK_EQ(zh[2], std::string("en-US"));
    }
}

TEST(tropmeta_languages_come_only_from_json_entries) {
    std::string bytes = fixture::MakeUcp({
        {"tropmeta_pt-BR.json", "{}"},
        {"tropmeta_es-ES.png", "x"},
        {"tropmeta_.json", "{}"},
        {"tropmeta_x", "x"},
        {"tropconf.json", "{}"},
    });
    Ucp ucp;
    std::string err;
    CHECK(ucp.ParseTable(bytes, bytes.size(), &err));
    std::vector<std::string> langs = TropmetaLanguages(ucp);
    CHECK_EQ(langs.size(), size_t(1));
    if (!langs.empty()) CHECK_EQ(langs[0], std::string("pt-BR"));
}

TEST(progress_matches_the_console_and_ignores_platinum) {
    TrophySet wolverine;
    auto add = [&wolverine](Grade grade, int earned, int total) {
        for (int i = 0; i < total; ++i) {
            Trophy t;
            t.grade = grade;
            t.earned = i < earned;
            wolverine.trophies.push_back(t);
        }
    };
    add(Grade::Platinum, 0, 1);
    add(Grade::Gold, 0, 4);
    add(Grade::Silver, 3, 9);
    add(Grade::Bronze, 18, 28);
    CHECK_EQ(wolverine.Progress(), 34);

    for (Trophy &t : wolverine.trophies) t.earned = t.grade != Grade::Platinum;
    CHECK_EQ(wolverine.Progress(), 100);
}
