#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace playtrack::fs {
struct Entry {
    std::string name;
    bool is_dir = false;
    int64_t size = 0;
};

struct Stat {
    bool is_dir = false;
    int64_t size = 0;
    int64_t mtime = 0;
};

std::optional<Stat> StatPath(const std::string &path);
bool IsNonEmptyFile(const std::string &path);
std::vector<Entry> ListDir(const std::string &path);

bool ReadRange(const std::string &path, uint64_t offset, int64_t length, std::string *out,
               int64_t max_bytes = 64 << 20);
bool ReadFile(const std::string &path, std::string *out, int64_t max_bytes = 64 << 20);

bool WriteFileAtomic(const std::string &path, std::string_view data);
bool EnsureDir(const std::string &path);

}
