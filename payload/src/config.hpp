#pragma once

#include <string>

namespace openpsapp {
inline constexpr const char *kOfficialServer = "https://openpsapp.com";

struct Config {
    int port = 7777;
    std::string language = "auto";
    bool debug = false;
    std::string server;
};

Config LoadOrCreateConfig(const std::string &path, std::string *warning);

std::string ResolveLanguage(const std::string &configured);

std::string CloudServer(const Config &cfg);

}
