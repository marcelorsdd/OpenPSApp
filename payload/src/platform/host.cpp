#include "platform/platform.hpp"

#include <cstdio>
#include <cstdlib>

namespace openpsapp::platform {
const char *Name() { return "host"; }

bool Escalate(std::string *) { return true; }

void SetProcessName(const char *) {}

const std::string &Root() {
    static const std::string root = [] {
        const char *env = std::getenv("OPENPSAPP_ROOT");
        return std::string(env ? env : "");
    }();
    return root;
}

std::optional<User> ForegroundUser() {
    const char *env = std::getenv("OPENPSAPP_USER");
    if (!env || !*env) return std::nullopt;

    User user;
    user.id = static_cast<int32_t>(std::strtoul(env, nullptr, 16));
    char hex[16];
    std::snprintf(hex, sizeof(hex), "%08x", static_cast<uint32_t>(user.id));
    user.id_hex = hex;
    const char *name = std::getenv("OPENPSAPP_USER_NAME");
    user.name = name ? name : "host-user";
    return user;
}

void Notify(const std::string &message) { std::printf("[notify] %s\n", message.c_str()); }

int SystemLanguageId() {
    const char *env = std::getenv("OPENPSAPP_LANG_ID");
    if (!env || !*env) return -1;
    char *end = nullptr;
    long id = std::strtol(env, &end, 10);
    return *end == '\0' && id >= 0 ? static_cast<int>(id) : -1;
}

std::string AccountIdHex(const User &) {
    const char *env = std::getenv("OPENPSAPP_ACCOUNT_ID");
    return env ? env : "";
}

std::string DeviceIdHex() {
    const char *env = std::getenv("OPENPSAPP_DEVICE_ID");
    return env ? env : "";
}

bool InstallPkg(const std::string &path, std::string *err) {
    if (err) *err = "PKG install is console-only (" + path + ")";
    return false;
}

}
