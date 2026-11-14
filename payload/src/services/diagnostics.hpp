#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "services/trophies.hpp"

namespace openpsapp::diagnostics {
struct AppDbTable {
    std::string name;
    int64_t rows = 0;
    std::vector<std::string> columns;
    std::vector<std::vector<std::pair<std::string, std::string>>> sample;
};

struct AppDbReport {
    bool ok = false;
    std::string error;
    std::vector<std::string> tables;
    std::vector<AppDbTable> details;
};

struct TrophyFileOwner {
    std::string set_id;
    bool ok = false;
    std::string error;
    uint32_t user_id = 0;
    std::string account_id_hex;
    std::string device_id_hex;
};

AppDbReport InspectAppDb();
bool CopyAppDb(const std::string &dest, std::string *err);
std::vector<TrophyFileOwner> TrophyFileOwners(const TrophyReport &trophies);

}
