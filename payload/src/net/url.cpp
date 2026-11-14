#include "net/url.hpp"

namespace openpsapp::net {
std::optional<Url> ParseHttpsUrl(std::string_view url) {
    constexpr std::string_view kScheme = "https://";
    if (url.substr(0, kScheme.size()) != kScheme) return std::nullopt;
    std::string_view rest = url.substr(kScheme.size());
    size_t end = rest.find_first_of("/?");
    std::string_view authority = rest.substr(0, end);
    std::string_view target = end == std::string_view::npos ? std::string_view() : rest.substr(end);

    Url out;
    size_t colon = authority.rfind(':');
    std::string_view host = authority.substr(0, colon);
    if (host.empty() || host.find_first_of("[]@ ") != std::string_view::npos) return std::nullopt;
    out.host = std::string(host);
    if (colon != std::string_view::npos) {
        std::string_view port = authority.substr(colon + 1);
        if (port.empty() || port.size() > 5) return std::nullopt;
        unsigned value = 0;
        for (char c : port) {
            if (c < '0' || c > '9') return std::nullopt;
            value = value * 10 + unsigned(c - '0');
        }
        if (value == 0 || value > 65535) return std::nullopt;
        out.port = uint16_t(value);
    }
    out.target = target.empty() ? "/" : target[0] == '?' ? "/" + std::string(target) : std::string(target);
    return out;
}

}
