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

struct TrophyOwner {
    std::string account_id_hex;
    uint32_t user_id = 0;
    std::string device_id_hex;
};

bool ParseTrophyStates(std::string_view file, std::vector<TrophyState> *out, std::string *err);
bool ReadTrophyStates(const std::string &path, std::vector<TrophyState> *out, std::string *err);
bool ParseTrophyOwner(std::string_view file, TrophyOwner *out, std::string *err);

int64_t TrophyTimeToUnix(uint64_t ticks);

}
