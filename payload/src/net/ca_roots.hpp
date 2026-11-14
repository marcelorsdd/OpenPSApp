#pragma once

#include <string_view>
#include <vector>

namespace openpsapp::net {
extern const std::string_view kIsrgRootX1;
extern const std::string_view kIsrgRootX2;
#ifdef OPENPSAPP_DEV_CA
extern const std::string_view kDevCa;
#endif

std::vector<std::string_view> ProductionRoots();

}
