#include "services/hidden.hpp"

#include <mutex>
#include <set>

#include "cJSON.h"
#include "util/fs.hpp"
#include "util/json.hpp"
#include "util/log.hpp"

namespace openpsapp::hidden {
namespace {
std::mutex g_mu;
std::string g_path;
std::set<std::string> g_ids;

bool Save() {
    JsonWriter w;
    w.BeginObject().Key("titleIds").BeginArray();
    for (const std::string &id : g_ids) w.String(id);
    w.EndArray().EndObject();
    return fs::WriteFileAtomic(g_path, w.take() + "\n");
}
}

void Load(const std::string &data_dir) {
    std::lock_guard<std::mutex> lock(g_mu);
    g_path = data_dir + "/hidden.json";
    g_ids.clear();
    std::string text;
    if (!fs::ReadFile(g_path, &text, 256 * 1024)) return;
    cJSON *root = cJSON_ParseWithLength(text.data(), text.size());
    const cJSON *item = nullptr;
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(root, "titleIds")) {
        if (cJSON_IsString(item)) g_ids.insert(item->valuestring);
    }
    if (!root) log::Error("hidden: %s is not valid JSON; starting empty", g_path.c_str());
    cJSON_Delete(root);
}

bool Contains(const std::string &title_id) {
    std::lock_guard<std::mutex> lock(g_mu);
    return g_ids.count(title_id) != 0;
}

bool Set(const std::string &title_id, bool hide) {
    std::lock_guard<std::mutex> lock(g_mu);
    bool changed = hide ? g_ids.insert(title_id).second : g_ids.erase(title_id) != 0;
    if (!changed || Save()) return true;

    if (hide) g_ids.erase(title_id);
    else g_ids.insert(title_id);
    return false;
}

}
