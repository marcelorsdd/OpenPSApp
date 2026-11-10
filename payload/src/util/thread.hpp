#pragma once

#include <functional>

namespace openpsapp {
bool Spawn(std::function<void()> fn);

void InstallSignalStack();

}
