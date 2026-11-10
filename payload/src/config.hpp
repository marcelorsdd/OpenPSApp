#pragma once

#include <string>

namespace openpsapp {
struct Config {
    int port = 7777;
    std::string language = "auto";
    bool debug = false;
};

Config LoadOrCreateConfig(const std::string &path, std::string *warning);

std::string ResolveLanguage(const std::string &configured);

}
