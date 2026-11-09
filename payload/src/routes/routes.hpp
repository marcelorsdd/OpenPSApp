#pragma once

#include <cstdint>
#include <string>

#include "config.hpp"
#include "http/http.hpp"

namespace playtrack::routes {
struct Context {
    std::string version;
    std::string data_dir;
    Config config;
    std::string language;
};

void RegisterAll(http::Router &router, const Context &ctx);

void RegisterHealth(http::Router &router, const Context &ctx);
void RegisterProfile(http::Router &router, const Context &ctx);
void RegisterGames(http::Router &router, const Context &ctx);
void RegisterTrophies(http::Router &router, const Context &ctx);
void RegisterIcons(http::Router &router, const Context &ctx);
void RegisterDebug(http::Router &router, const Context &ctx);
void RegisterWeb(http::Router &router, const Context &ctx);

bool ServedByPlayTrack(uint16_t port);

}
