#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "formats/npbind.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string_view input(reinterpret_cast<const char *>(data), size);
    std::string id = openpsapp::ParseNpCommId(input);
    volatile size_t sink = id.size();
    (void)sink;
    if (!id.empty()) {
        volatile bool ok = openpsapp::IsNpCommId(id);
        (void)ok;
    }
    return 0;
}
