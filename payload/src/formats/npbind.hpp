#pragma once

#include <string>
#include <string_view>

namespace openpsapp {
std::string ParseNpCommId(std::string_view data);

bool IsNpCommId(std::string_view id);

}
