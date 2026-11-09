#include "services/icons.hpp"

#include <mutex>
#include <unordered_map>

#include "util/fs.hpp"
#include "util/sha256.hpp"

namespace playtrack::icons {
namespace {
std::mutex g_mu;
std::unordered_map<std::string, Location> g_by_id;
}

std::string FileVersion(const std::string &path) {
    auto st = fs::StatPath(path);
    return st && !st->is_dir ? std::to_string(st->size) + "-" + std::to_string(st->mtime) : "";
}

std::string Register(const Location &where) {
    if (where.path.empty()) return "";
    std::string key = where.path + "#" + std::to_string(where.offset) + "+" + std::to_string(where.length) + "@" +
                      where.version;
    std::string id = Sha256::Hex(key).substr(0, 32);
    std::lock_guard<std::mutex> lock(g_mu);
    g_by_id.emplace(id, where);
    return id;
}

std::optional<Location> Find(const std::string &id) {
    std::lock_guard<std::mutex> lock(g_mu);
    auto it = g_by_id.find(id);
    if (it == g_by_id.end()) return std::nullopt;
    return it->second;
}

std::string Url(const Location &where) {
    std::string id = Register(where);
    return id.empty() ? "" : "/api/v1/icons/" + id;
}

}
