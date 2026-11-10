#include "formats/npbind.hpp"

#include <cctype>

#include "util/bytes.hpp"

namespace openpsapp {
namespace {
constexpr uint32_t kMagic = 0xd294a018;
constexpr size_t kRecordsOffset = 0x80;
constexpr uint16_t kNpCommIdTag = 0x0010;
}

bool IsNpCommId(std::string_view id) {
    if (id.size() != 12 || id.substr(0, 4) != "NPWR" || id[9] != '_') return false;
    for (size_t i : {4, 5, 6, 7, 8, 10, 11}) {
        if (!std::isdigit(static_cast<unsigned char>(id[i]))) return false;
    }
    return true;
}

std::string ParseNpCommId(std::string_view data) {
    if (data.size() < kRecordsOffset + 4 || bytes::Be32(data, 0) != kMagic) return "";
    size_t pos = kRecordsOffset;
    while (pos + 4 <= data.size()) {
        uint16_t tag = bytes::Be16(data, pos);
        uint16_t len = bytes::Be16(data, pos + 2);
        if (pos + 4 + len > data.size()) break;
        if (tag == kNpCommIdTag) {
            std::string_view value = data.substr(pos + 4, len);
            value = value.substr(0, value.find('\0'));
            return IsNpCommId(value) ? std::string(value) : "";
        }
        pos += 4 + len;
    }
    return "";
}

}
