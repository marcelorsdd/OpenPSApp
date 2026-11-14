#pragma once

#include "cloud/controller.hpp"
#include "routes/routes.hpp"

namespace openpsapp::cloud {
CloudController *Controller();

void Start(const routes::Context &ctx);

}
