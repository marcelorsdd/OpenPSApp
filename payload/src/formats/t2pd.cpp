#include "formats/t2pd.hpp"

#include <cstring>

#include "util/bytes.hpp"
#include "util/fs.hpp"
#include "util/sha256.hpp"

namespace playtrack {
namespace {
constexpr size_t kHeaderSize = 64;
constexpr size_t kPageData = 1024;
constexpr size_t kPageStride = kPageData + 32;
constexpr uint32_t kVersion = 0x10000;
constexpr size_t kStreamHeader = 48;
constexpr size_t kDirEntrySize = 32;
constexpr uint32_t kStateTag = 0x800;
constexpr uint32_t kEarnedFlags = 0x11;
constexpr int64_t kEpochOffsetSeconds = 62135596800LL;

bool AllZero(std::string_view b) {
    for (char c : b) {
        if (c != 0) return false;
    }
    return true;
}
}

int64_t TrophyTimeToUnix(uint64_t ticks) {
    return ticks ? static_cast<int64_t>(ticks / 1000000) - kEpochOffsetSeconds : 0;
}

bool ParseTrophyStates(std::string_view file, std::vector<TrophyState> *out, std::string *err) {
    out->clear();
    if (file.size() < kHeaderSize || file.substr(0, 4) != "T2PD" || bytes::Be32(file, 4) != kVersion ||
        bytes::Be64(file, 8) != kPageStride) {
        *err = "not a T2PD file";
        return false;
    }
    uint64_t pages = bytes::Be64(file, 32);
    if (pages == 0 || pages > (16 << 20) / kPageStride || file.size() != kHeaderSize + pages * kPageStride) {
        *err = "T2PD page count does not match file size";
        return false;
    }

    std::string stream;
    stream.reserve(static_cast<size_t>(pages) * kPageData);
    for (uint64_t p = 0; p < pages; ++p) {
        std::string_view data = file.substr(kHeaderSize + p * kPageStride, kPageData);
        std::string_view digest = file.substr(kHeaderSize + p * kPageStride + kPageData, 32);
        Sha256 sha;
        sha.Update(data.data(), data.size());
        auto actual = sha.Digest();
        bool ok = std::memcmp(actual.data(), digest.data(), 32) == 0 || (AllZero(digest) && AllZero(data));
        if (!ok) {
            *err = "T2PD page " + std::to_string(p) + " fails its SHA-256";
            return false;
        }
        stream.append(data);
    }

    if (stream.substr(0, 4) != "T2TD" || bytes::Be32(stream, 4) != kVersion) {
        *err = "missing T2TD stream";
        return false;
    }
    uint32_t dir_count = bytes::Be32(stream, 8);
    if (dir_count == 0 || dir_count > 64 || kStreamHeader + dir_count * kDirEntrySize > stream.size()) {
        *err = "T2TD directory out of range";
        return false;
    }

    for (uint32_t i = 0; i < dir_count; ++i) {
        size_t d = kStreamHeader + i * kDirEntrySize;
        if (bytes::Be32(stream, d) != kStateTag) continue;
        uint32_t payload = bytes::Be32(stream, d + 4);
        uint32_t version = bytes::Be32(stream, d + 8);
        uint32_t repeat = bytes::Be32(stream, d + 12);
        uint64_t offset = bytes::Be64(stream, d + 16);
        uint64_t stride = 16 + uint64_t(payload);
        if (version != 1 || payload < 0x30 || repeat == 0 || repeat > 4096 || offset > stream.size() ||
            repeat * stride > stream.size() - offset) {
            *err = "T2TD state table out of range";
            return false;
        }
        for (uint32_t r = 0; r < repeat; ++r) {
            size_t rec = static_cast<size_t>(offset + r * stride);
            if (bytes::Be32(stream, rec) != kStateTag) {
                *err = "T2TD state record " + std::to_string(r) + " has wrong tag";
                out->clear();
                return false;
            }
            TrophyState s;
            s.id = static_cast<int>(bytes::Be32(stream, rec + 0x10));
            uint32_t flags = bytes::Be32(stream, rec + 0x14);
            uint64_t earned_time = bytes::Be64(stream, rec + 0x20);
            uint64_t second_time = bytes::Be64(stream, rec + 0x30);
            s.earned = flags == kEarnedFlags && earned_time != 0 && second_time != 0;
            s.earned_at = s.earned ? TrophyTimeToUnix(earned_time) : 0;
            out->push_back(s);
        }
        return true;
    }
    *err = "T2TD has no state table";
    return false;
}

bool ReadTrophyStates(const std::string &path, std::vector<TrophyState> *out, std::string *err) {
    std::string file;
    if (!fs::ReadFile(path, &file, 16 << 20)) {
        *err = "read " + path;
        return false;
    }
    return ParseTrophyStates(file, out, err);
}

}
