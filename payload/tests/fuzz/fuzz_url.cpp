#include <cstddef>
#include <cstdint>
#include <string_view>

#include "net/url.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string_view input(reinterpret_cast<const char *>(data), size);
    auto url = openpsapp::net::ParseHttpsUrl(input);
    if (url) {
        volatile size_t sink = url->host.size() + url->target.size() + url->port;
        (void)sink;
    }
    return 0;
}
