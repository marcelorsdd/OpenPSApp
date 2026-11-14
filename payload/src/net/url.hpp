#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace openpsapp::net {
struct Url {
    std::string host;
    uint16_t port = 443;
    std::string target;
};

std::optional<Url> ParseHttpsUrl(std::string_view url);

}
