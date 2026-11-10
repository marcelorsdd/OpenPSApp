#pragma once

#include <string>

namespace openpsapp::tile {
extern const unsigned char kPkg[];
extern const unsigned kPkgSize;
extern const char kTitleId[];
extern const int kDeeplinkPort;

struct Status {
    bool embedded = false;
    bool installed = false;
    std::string state;
    std::string error;
};

Status GetStatus();
void InstallIfNeeded(const std::string &data_dir, int port, const std::string &language);

}
