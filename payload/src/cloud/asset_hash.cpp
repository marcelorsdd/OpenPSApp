#include "cloud/asset_hash.hpp"

#include "cJSON.h"
#include "util/fs.hpp"
#include "util/json.hpp"
#include "util/sha256.hpp"

namespace openpsapp::cloud {
namespace {
constexpr int64_t kMaxImageBytes = 32 << 20;
}

AssetHasher::AssetHasher(std::string cache_path) : cache_path_(std::move(cache_path)) {
    std::string text;
    if (!fs::ReadFile(cache_path_, &text, 8 << 20)) return;
    cJSON *root = cJSON_ParseWithLength(text.data(), text.size());
    const cJSON *item = nullptr;
    cJSON_ArrayForEach(item, root) {
        if (cJSON_IsString(item) && item->string) hashes_[item->string] = item->valuestring;
    }
    cJSON_Delete(root);
}

std::string AssetHasher::Hash(const icons::Location &where) {
    if (where.path.empty()) return "";
    std::string id = icons::Register(where);
    auto it = hashes_.find(id);
    if (it != hashes_.end()) return it->second;
    std::string bytes;
    if (!fs::ReadRange(where.path, where.offset, where.length, &bytes, kMaxImageBytes)) return "";
    std::string hash = Sha256::Hex(bytes);
    hashes_[id] = hash;
    dirty_ = true;
    return hash;
}

void AssetHasher::Flush() {
    if (!dirty_) return;
    JsonWriter w;
    w.BeginObject();
    for (const auto &[id, hash] : hashes_) w.String(id, hash);
    w.EndObject();
    if (fs::WriteFileAtomic(cache_path_, w.take() + "\n")) dirty_ = false;
}

}
