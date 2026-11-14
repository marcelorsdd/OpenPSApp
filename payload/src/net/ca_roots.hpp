#pragma once

#include <string_view>
#include <vector>

namespace openpsapp::net {
extern const std::string_view kIsrgRootX1;
extern const std::string_view kIsrgRootX2;

std::vector<std::string_view> ProductionRoots();

}
