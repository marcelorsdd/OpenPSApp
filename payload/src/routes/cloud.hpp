#pragma once

#include <functional>

#include "http/http.hpp"

namespace openpsapp::cloud {
class CloudController;
}

namespace openpsapp::routes {
bool FromConsole(const http::Request &req, int port);
void RegisterCloud(http::Router &router, int port, std::function<cloud::CloudController *()> controller);

}
