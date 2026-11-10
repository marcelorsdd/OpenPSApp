#include "util/timeutil.hpp"

#include <sys/time.h>

#include <cctype>
#include <ctime>

namespace openpsapp::timeutil {
int64_t NowUnix() { return static_cast<int64_t>(time(nullptr)); }

int64_t NowMillis() {
    timeval tv {};
    gettimeofday(&tv, nullptr);
    return static_cast<int64_t>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

std::string FormatIso(int64_t unix_seconds) {
    time_t t = static_cast<time_t>(unix_seconds);
    tm parts {};
    gmtime_r(&t, &parts);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &parts);
    return buf;
}

std::string NowIso() { return FormatIso(NowUnix()); }

std::string FormatIsoOrEmpty(int64_t unix_seconds) { return unix_seconds ? FormatIso(unix_seconds) : ""; }

std::string NormalizeTimestamp(std::string_view raw) {
    if (raw.empty()) return "";

    bool all_digits = true;
    for (char c : raw) all_digits = all_digits && std::isdigit(static_cast<unsigned char>(c));
    if (all_digits) {
        int64_t value = std::stoll(std::string(raw));
        if (value <= 0) return "";
        if (value > 100000000000LL) value /= 1000;
        return FormatIso(value);
    }

    if (raw.size() >= 19 && raw[4] == '-' && raw[7] == '-' && (raw[10] == ' ' || raw[10] == 'T') &&
        raw[13] == ':' && raw[16] == ':') {
        std::string out(raw.substr(0, 19));
        out[10] = 'T';
        if (out.rfind("0000-", 0) == 0) return "";
        return out + "Z";
    }
    return "";
}

}
