#pragma once

#include <optional>

#include "platform/platform.hpp"
#include "services/library.hpp"
#include "util/json.hpp"

namespace openpsapp::routes {
void WriteUser(JsonWriter &w, const std::optional<platform::User> &user);
void WriteGame(JsonWriter &w, const Library &lib, const GamePlaytime &game);
void WriteGradeCounts(JsonWriter &w, const Tally &tally);
void WriteTrophySet(JsonWriter &w, const Library &lib, const TrophySet &set);
void WriteTrophy(JsonWriter &w, const Trophy &trophy);

}
