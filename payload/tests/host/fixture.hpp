#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "platform/platform.hpp"

namespace fixture {
constexpr const char *kUserHex = "1eb71bbd";
constexpr int32_t kUserId = 0x1eb71bbd;
constexpr const char *kSetId = "NPWR00001_00";
constexpr const char *kCorruptSetId = "NPWR00002_00";
constexpr const char *kAccountIdHex = "1122334455667788";
constexpr const char *kDeviceIdHex = "00112233445566778899aabbccddeeff";

struct StateSpec {
    int id;
    uint32_t flags;
    int64_t earned_unix;
    int64_t second_unix;
};

std::string MakeUcp(const std::vector<std::pair<std::string, std::string>> &entries);
std::string MakeT2pd(const std::vector<StateSpec> &states);
std::string MakeNpbind(const std::string &np_comm_id);

bool Build(const std::string &root, std::string *err);

inline openpsapp::platform::User User() {
    openpsapp::platform::User user;
    user.id = kUserId;
    user.id_hex = kUserHex;
    user.name = "tester";
    return user;
}

}
