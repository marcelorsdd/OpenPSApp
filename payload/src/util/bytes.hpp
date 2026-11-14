#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace openpsapp::bytes {
inline uint16_t Be16(std::string_view b, size_t off) {
    return static_cast<uint16_t>((uint8_t(b[off]) << 8) | uint8_t(b[off + 1]));
}

inline uint32_t Be32(std::string_view b, size_t off) {
    return (uint32_t(uint8_t(b[off])) << 24) | (uint32_t(uint8_t(b[off + 1])) << 16) |
           (uint32_t(uint8_t(b[off + 2])) << 8) | uint32_t(uint8_t(b[off + 3]));
}

inline uint64_t Be64(std::string_view b, size_t off) { return (uint64_t(Be32(b, off)) << 32) | Be32(b, off + 4); }

inline std::string Hex(std::string_view b) {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(b.size() * 2);
    for (unsigned char c : b) {
        out += kDigits[c >> 4];
        out += kDigits[c & 0xf];
    }
    return out;
}

}
