#pragma once

#include <map>
#include <string>

#include "services/icons.hpp"

namespace openpsapp::cloud {
class AssetHasher {
public:
    explicit AssetHasher(std::string cache_path);

    std::string Hash(const icons::Location &where);
    void Flush();

private:
    std::string cache_path_;
    std::map<std::string, std::string> hashes_;
    bool dirty_ = false;
};

}
