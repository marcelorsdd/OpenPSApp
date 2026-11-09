#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "platform/platform.hpp"
#include "services/playtime.hpp"
#include "services/trophies.hpp"

namespace playtrack {
struct ProfileTotals {
    int64_t games = 0;
    int64_t playtime_seconds = 0;
    Tally trophies;
};

struct EarnedTrophy {
    const TrophySet *set = nullptr;
    const Trophy *trophy = nullptr;
};

struct Library {
    std::optional<platform::User> user;
    PlaytimeReport playtime;
    TrophyReport trophies;
    std::map<std::string, std::string> set_for_title;
    std::map<std::string, std::string> title_for_set;

    const GamePlaytime *FindGame(const std::string &title_id) const { return playtime.Find(title_id); }
    const TrophySet *FindSet(const std::string &set_id) const { return trophies.Find(set_id); }
    ProfileTotals Totals() const;
    std::vector<EarnedTrophy> EarnedTrophies(size_t limit) const;
};

std::shared_ptr<const Library> CurrentLibrary(const std::string &language);

constexpr int kLibraryTtlSec = 30;

std::string TrophySetForTitle(const std::string &title_id, const std::string &metadata_dir);

}
