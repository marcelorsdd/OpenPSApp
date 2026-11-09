#pragma once

#include <string>

namespace playtrack::layout {
constexpr const char *kAppDb = "/system_data/priv/mms/app.db";

constexpr const char *kIconInfoPrefix = "tbl_iconinfo_";

constexpr const char *kTrophyConfDir = "/user/trophy2/nobackup/conf";
constexpr const char *kHomeDir = "/user/home";

inline std::string TrophyDataDir(const std::string &user_hex) {
    return std::string(kHomeDir) + "/" + user_hex + "/trophy2/nobackup/data";
}

inline std::string AppMetaDir(const std::string &title_id) { return "/user/appmeta/" + title_id; }

inline std::string InternalSceSys(const std::string &title_id) { return "/user/app/" + title_id + "/sce_sys"; }

}
