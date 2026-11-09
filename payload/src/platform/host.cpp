#include "platform/platform.hpp"

#include <cstdio>
#include <cstdlib>

namespace playtrack::platform {
const char *Name() { return "host"; }

bool Escalate(std::string *) { return true; }

void SetProcessName(const char *) {}

const std::string &Root() {
    static const std::string root = [] {
        const char *env = std::getenv("PLAYTRACK_ROOT");
        return std::string(env ? env : "");
    }();
    return root;
}

std::optional<User> ForegroundUser() {
    const char *env = std::getenv("PLAYTRACK_USER");
    if (!env || !*env) return std::nullopt;

    User user;
    user.id = static_cast<int32_t>(std::strtoul(env, nullptr, 16));
    char hex[16];
    std::snprintf(hex, sizeof(hex), "%08x", static_cast<uint32_t>(user.id));
    user.id_hex = hex;
    const char *name = std::getenv("PLAYTRACK_USER_NAME");
    user.name = name ? name : "host-user";
    return user;
}

void Notify(const std::string &message) { std::printf("[notify] %s\n", message.c_str()); }

bool InstallPkg(const std::string &path, std::string *err) {
    if (err) *err = "PKG install is console-only (" + path + ")";
    return false;
}

}
