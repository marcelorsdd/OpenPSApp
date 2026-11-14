#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "net/http_message.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string_view raw(reinterpret_cast<const char *>(data), size);
    int status = 0;
    std::string body;
    std::string err;
    if (openpsapp::net::ParseResponse(raw, &status, &body, &err)) {
        volatile size_t sink = body.size() + static_cast<size_t>(status);
        (void)sink;
    }
    return 0;
}
