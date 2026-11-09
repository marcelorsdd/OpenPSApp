#pragma once

#include <string>

namespace playtrack {
struct Config {
    int port = 7777;
    std::string language = "pt-BR";
    bool debug = false;
};

Config LoadOrCreateConfig(const std::string &path, std::string *warning);

}
