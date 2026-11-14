#include "cloud/store.hpp"

#include <algorithm>

#include "cJSON.h"
#include "util/fs.hpp"
#include "util/json.hpp"

namespace openpsapp::cloud {
CloudStore::CloudStore(std::string path) : path_(std::move(path)) {
    std::string text;
    if (!fs::ReadFile(path_, &text, 1 << 20)) return;
    cJSON *root = cJSON_ParseWithLength(text.data(), text.size());
    const cJSON *links = cJSON_GetObjectItemCaseSensitive(root, "links");
    const cJSON *item = nullptr;
    cJSON_ArrayForEach(item, links) {
        if (!cJSON_IsObject(item) || !item->string) continue;
        CloudLink link{JsonString(item, "token"), JsonString(item, "deviceId"), JsonString(item, "profileName")};
        if (!link.token.empty()) links_[item->string] = link;
    }
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(root, "revoke")) {
        if (cJSON_IsString(item) && *item->valuestring) revoke_.push_back(item->valuestring);
    }
    cJSON_Delete(root);
}

std::optional<CloudLink> CloudStore::Get(const std::string &user) const {
    auto it = links_.find(user);
    if (it == links_.end()) return std::nullopt;
    return it->second;
}

bool CloudStore::Put(const std::string &user, const CloudLink &link) {
    links_[user] = link;
    return Save();
}

bool CloudStore::Remove(const std::string &user) {
    links_.erase(user);
    return Save();
}

std::optional<std::string> CloudStore::Retire(const std::string &user) {
    auto it = links_.find(user);
    if (it == links_.end()) return std::nullopt;
    std::string token = it->second.token;
    links_.erase(it);
    if (std::find(revoke_.begin(), revoke_.end(), token) == revoke_.end()) revoke_.push_back(token);
    Save();
    return token;
}

bool CloudStore::Revoked(const std::string &token) {
    auto it = std::find(revoke_.begin(), revoke_.end(), token);
    if (it == revoke_.end()) return true;
    revoke_.erase(it);
    return Save();
}

bool CloudStore::Save() const {
    JsonWriter w;
    w.BeginObject().BeginObject("links");
    for (const auto &[user, link] : links_) {
        w.BeginObject(user)
            .String("token", link.token)
            .String("deviceId", link.device_id)
            .String("profileName", link.profile_name)
            .EndObject();
    }
    w.EndObject().BeginArray("revoke");
    for (const std::string &token : revoke_) w.String(token);
    w.EndArray().EndObject();
    return fs::WriteFileAtomic(path_, w.take() + "\n", 0600);
}

}
