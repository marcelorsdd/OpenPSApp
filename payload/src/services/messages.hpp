#pragma once

#include <string>

namespace openpsapp::messages {

enum class Id { Started, AlreadyRunning, PortBusy, TileInstalled };

std::string Text(Id id, const std::string &language, int port);

}
