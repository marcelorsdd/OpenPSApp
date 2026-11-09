#include "fixture.hpp"
#include "services/trophies.hpp"
#include "test.hpp"

using namespace playtrack;

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

    CHECK_EQ(set->Progress(), 33);
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
