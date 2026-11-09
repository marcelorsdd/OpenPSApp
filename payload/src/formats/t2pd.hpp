#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace playtrack {
struct TrophyState {
    int id = 0;
    bool earned = false;
    int64_t earned_at = 0;
};

bool ParseTrophyStates(std::string_view file, std::vector<TrophyState> *out, std::string *err);
bool ReadTrophyStates(const std::string &path, std::vector<TrophyState> *out, std::string *err);

int64_t TrophyTimeToUnix(uint64_t ticks);

}
