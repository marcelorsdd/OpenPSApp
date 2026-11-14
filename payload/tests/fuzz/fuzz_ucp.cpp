#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "formats/ucp.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string_view head(reinterpret_cast<const char *>(data), size);
    openpsapp::Ucp ucp;
    std::string err;
    if (ucp.ParseTable(head, size, &err)) {
        for (const auto &e : ucp.entries()) {
            volatile uint64_t sink = e.offset + e.size + e.name.size();
            (void)sink;
        }
        if (!ucp.entries().empty()) ucp.Find(ucp.entries().front().name);
    }
    return 0;
}
