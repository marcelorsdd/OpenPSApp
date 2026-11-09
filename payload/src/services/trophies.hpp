#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "formats/ucp.hpp"
#include "platform/platform.hpp"
#include "services/icons.hpp"

namespace playtrack {
enum class Grade { Platinum, Gold, Silver, Bronze, Unknown };
constexpr std::array<Grade, 4> kGrades = {Grade::Platinum, Grade::Gold, Grade::Silver, Grade::Bronze};

const char *GradeName(Grade grade);

int GradePoints(Grade grade);

struct Trophy {
    int id = 0;
    std::string name;
    std::string detail;
    Grade grade = Grade::Unknown;
    bool hidden = false;
    bool earned = false;
    int64_t earned_at = 0;
    icons::Location icon;
};

struct GradeTally {
    int earned = 0;
    int total = 0;
};

struct Tally {
    std::array<GradeTally, 4> by_grade;
    int earned = 0;
    int total = 0;

    void Add(const Tally &other);
};

struct TrophySet {
    std::string id;
    std::string name;
    icons::Location icon;
    std::vector<Trophy> trophies;

    Tally Count() const;
    int Progress() const;
    int64_t LastEarnedAt() const;
};

struct TrophyReport {
    std::string data_dir;
    std::vector<TrophySet> sets;
    std::vector<std::string> errors;

    const TrophySet *Find(const std::string &set_id) const;
};

TrophyReport CollectTrophies(const std::optional<platform::User> &user, const std::string &language);

std::vector<std::string> TropmetaLanguages(const Ucp &ucp);
std::vector<std::string> LanguageCandidates(const std::string &language, const std::vector<std::string> &available,
                                            const std::string &default_language);

bool BuildTrophySet(const std::string &id, const std::string &ucp_path, const std::string &dat_path,
                    const std::string &language, TrophySet *out, std::string *err);

}
