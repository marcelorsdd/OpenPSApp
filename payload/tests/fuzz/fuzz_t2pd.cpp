#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "formats/t2pd.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string_view file(reinterpret_cast<const char *>(data), size);
    std::string err;

    std::vector<openpsapp::TrophyState> states;
    if (openpsapp::ParseTrophyStates(file, &states, &err)) {
        for (const auto &s : states) {
            volatile int64_t sink = s.id + s.earned_at + (s.earned ? 1 : 0);
            (void)sink;
        }
    }

    openpsapp::TrophyOwner owner;
    if (openpsapp::ParseTrophyOwner(file, &owner, &err)) {
        volatile size_t sink = owner.account_id_hex.size() + owner.device_id_hex.size() + owner.user_id;
        (void)sink;
    }
    return 0;
}
