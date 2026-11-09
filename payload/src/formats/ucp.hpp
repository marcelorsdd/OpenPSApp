#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace playtrack {
class Ucp {
public:
    struct Entry {
        std::string name;
        uint64_t offset = 0;
        uint64_t size = 0;
    };

    bool Open(const std::string &path, std::string *err);

    bool ParseTable(std::string_view head, uint64_t file_size, std::string *err);

    const Entry *Find(std::string_view name) const;
    bool Read(const Entry &entry, std::string *out, uint64_t max_bytes = 4 << 20) const;

    const std::string &path() const { return path_; }
    const std::vector<Entry> &entries() const { return entries_; }

    static constexpr uint32_t kMagic = 0xb228c60a;
    static constexpr size_t kTableOffset = 0x60;
    static constexpr size_t kEntrySize = 64;
    static constexpr uint32_t kMaxEntries = 4096;

private:
    std::string path_;
    std::vector<Entry> entries_;
};

}
