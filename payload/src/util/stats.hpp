#pragma once

#include <atomic>
#include <cstdint>

namespace playtrack::stats {
struct Stats {
    int64_t started_at = 0;
    std::atomic<int64_t> requests{0};
    std::atomic<int64_t> collections{0};
    std::atomic<int64_t> last_collect_ms{0};
};

inline Stats &Get() {
    static Stats stats;
    return stats;
}

}
