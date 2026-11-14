#pragma once

#include <functional>
#include <map>
#include <string>

#include "services/icons.hpp"
#include "services/library.hpp"

namespace openpsapp::cloud {
struct Snapshot {
    std::string manifest;
    std::map<std::string, std::string> sets;
    std::map<std::string, icons::Location> assets;
    std::string fingerprint;
};

using AssetHashFn = std::function<std::string(const icons::Location &)>;

Snapshot BuildSnapshot(const Library &lib, const std::string &profile_name, const std::string &language,
                       const AssetHashFn &hash);

}
