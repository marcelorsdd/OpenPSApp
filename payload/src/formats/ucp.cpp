#include "formats/ucp.hpp"

#include "util/bytes.hpp"
#include "util/fs.hpp"

namespace openpsapp {
bool Ucp::ParseTable(std::string_view head, uint64_t file_size, std::string *err) {
    entries_.clear();
    if (head.size() < kTableOffset || bytes::Be32(head, 0) != kMagic) {
        *err = "not a UCP (bad magic)";
        return false;
    }
    uint32_t count = bytes::Be32(head, 0x10);
    uint32_t entry_size = bytes::Be32(head, 0x14);
    if (entry_size != kEntrySize || count == 0 || count > kMaxEntries) {
        *err = "UCP table header out of range";
        return false;
    }
    if (head.size() < kTableOffset + count * kEntrySize) {
        *err = "UCP table truncated";
        return false;
    }

    for (uint32_t i = 0; i < count; ++i) {
        size_t off = kTableOffset + i * kEntrySize;
        std::string_view raw = head.substr(off, 32);
        Entry e;
        e.name = std::string(raw.substr(0, raw.find('\0')));
        e.offset = bytes::Be64(head, off + 32);
        e.size = bytes::Be64(head, off + 40);
        bool printable = !e.name.empty();
        for (char c : e.name) printable = printable && c > 0x20 && c < 0x7f && c != '/' && c != '\\';
        if (!printable || e.offset > file_size || e.size > file_size - e.offset) {
            *err = "UCP entry " + std::to_string(i) + " invalid";
            entries_.clear();
            return false;
        }
        entries_.push_back(std::move(e));
    }
    return true;
}

bool Ucp::Open(const std::string &path, std::string *err) {
    path_ = path;
    auto st = fs::StatPath(path);
    std::string head;
    if (!st || !fs::ReadRange(path, 0, kTableOffset, &head)) {
        *err = "read " + path;
        return false;
    }

    uint32_t count = bytes::Be32(head, 0) == kMagic ? bytes::Be32(head, 0x10) : 0;
    std::string table;
    if (count > 0 && count <= kMaxEntries) {
        if (!fs::ReadRange(path, kTableOffset, count * kEntrySize, &table)) {
            *err = "read " + path;
            return false;
        }
        head += table;
    }
    return ParseTable(head, static_cast<uint64_t>(st->size), err);
}

const Ucp::Entry *Ucp::Find(std::string_view name) const {
    for (const Entry &e : entries_) {
        if (e.name == name) return &e;
    }
    return nullptr;
}

bool Ucp::Read(const Entry &entry, std::string *out, uint64_t max_bytes) const {
    return entry.size <= max_bytes &&
           fs::ReadRange(path_, entry.offset, static_cast<int64_t>(entry.size), out, static_cast<int64_t>(max_bytes));
}

}
