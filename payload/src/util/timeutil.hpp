#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace playtrack::timeutil {
int64_t NowUnix();
int64_t NowMillis();

std::string FormatIso(int64_t unix_seconds);
std::string NowIso();
std::string FormatIsoOrEmpty(int64_t unix_seconds);

std::string NormalizeTimestamp(std::string_view raw);

}
