#pragma once

#include <functional>

namespace playtrack {
bool Spawn(std::function<void()> fn);

void InstallSignalStack();

}
