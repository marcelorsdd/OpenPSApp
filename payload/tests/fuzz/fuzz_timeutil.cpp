#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "util/timeutil.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string_view raw(reinterpret_cast<const char *>(data), size);
    std::string out = openpsapp::timeutil::NormalizeTimestamp(raw);
    volatile size_t sink = out.size();
    (void)sink;

    if (size >= 8) {
        int64_t seconds = 0;
        for (size_t i = 0; i < 8; ++i) seconds = (seconds << 8) | data[i];
        std::string iso = openpsapp::timeutil::FormatIso(seconds);
        volatile size_t sink2 = iso.size();
        (void)sink2;
    }
    return 0;
}
