#pragma once

#include <cstdint>
#include <string>

namespace playtrack::http {
bool LocalGet(uint16_t port, const std::string &path, std::string *body, int timeout_ms = 1500);

}
