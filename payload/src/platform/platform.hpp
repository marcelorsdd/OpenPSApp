#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace openpsapp::platform {
struct User {
    int32_t id = 0;
    std::string id_hex;
    std::string name;
};

const char *Name();

bool Escalate(std::string *err);

void SetProcessName(const char *name);

const std::string &Root();

inline std::string ConsolePath(std::string_view console_path) { return Root() + std::string(console_path); }

std::optional<User> ForegroundUser();

void Notify(const std::string &message);

int SystemLanguageId();

std::string AccountIdHex(const User &user);
std::string DeviceIdHex();

bool InstallPkg(const std::string &path, std::string *err);

}
