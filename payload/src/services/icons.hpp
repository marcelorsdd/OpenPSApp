#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace openpsapp::icons {
struct Location {
    std::string path;
    uint64_t offset = 0;
    int64_t length = -1;
    std::string version;
};

std::string FileVersion(const std::string &path);

std::string Register(const Location &where);
std::optional<Location> Find(const std::string &id);

std::string Url(const Location &where);

}
