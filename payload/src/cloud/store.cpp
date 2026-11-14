#include "cloud/store.hpp"

#include <sys/stat.h>

#include "cJSON.h"
#include "util/fs.hpp"
#include "util/json.hpp"

namespace openpsapp::cloud {
namespace {
std::string StringField(const cJSON *obj, const char *name) {
    const cJSON *v = cJSON_GetObjectItemCaseSensitive(obj, name);
    return cJSON_IsString(v) ? v->valuestring : "";
}
}

CloudStore::CloudStore(std::string path) : path_(std::move(path)) {
    std::string text;
    if (!fs::ReadFile(path_, &text, 1 << 20)) return;
    cJSON *root = cJSON_ParseWithLength(text.data(), text.size());
    const cJSON *links = cJSON_GetObjectItemCaseSensitive(root, "links");
    const cJSON *item = nullptr;
    cJSON_ArrayForEach(item, links) {
        if (!cJSON_IsObject(item) || !item->string) continue;
        CloudLink link{StringField(item, "token"), StringField(item, "deviceId"), StringField(item, "profileName")};
        if (!link.token.empty()) links_[item->string] = link;
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

bool CloudStore::Save() const {
    JsonWriter w;
    w.BeginObject().Key("links").BeginObject();
    for (const auto &[user, link] : links_) {
        w.Key(user)
            .BeginObject()
            .Key("token").String(link.token)
            .Key("deviceId").String(link.device_id)
            .Key("profileName").String(link.profile_name)
            .EndObject();
    }
    w.EndObject().EndObject();
    if (!fs::WriteFileAtomic(path_, w.take() + "\n")) return false;
    return chmod(path_.c_str(), 0600) == 0;
}

}
