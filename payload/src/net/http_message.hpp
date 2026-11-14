#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "net/url.hpp"

namespace openpsapp::net {
using Headers = std::vector<std::pair<std::string, std::string>>;

std::string BuildRequest(std::string_view method, const Url &url, const Headers &headers, std::string_view body);

bool ParseResponse(std::string_view raw, int *status, std::string *body, std::string *err);

}
