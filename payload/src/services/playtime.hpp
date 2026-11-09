#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "platform/platform.hpp"
#include "services/icons.hpp"

namespace playtrack {
struct GamePlaytime {
    std::string title_id;
    std::string name;
    int64_t playtime_seconds = 0;
    std::string last_played_at;
    icons::Location icon;
    icons::Location hero;
    std::string metadata_dir;
};

struct PlaytimeReport {
    std::vector<GamePlaytime> games;
    std::vector<std::string> tables;
    std::string error;

    const GamePlaytime *Find(const std::string &title_id) const;
};

PlaytimeReport CollectPlaytime(const std::optional<platform::User> &user);

bool IsGameTitleId(std::string_view title_id);

}
