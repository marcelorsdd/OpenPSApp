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

inline playtrack::platform::User User() {
    playtrack::platform::User user;
    user.id = kUserId;
    user.id_hex = kUserHex;
    user.name = "tester";
    return user;
}

}
