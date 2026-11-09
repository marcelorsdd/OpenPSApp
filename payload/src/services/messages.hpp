#pragma once

#include <string>

namespace playtrack::messages {

enum class Id { Started, AlreadyRunning, PortBusy, TileInstalled };

std::string Text(Id id, const std::string &language, int port);

}
