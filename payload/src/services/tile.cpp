#include "services/tile.hpp"

#include <unistd.h>

#include <mutex>

#include "platform/platform.hpp"
#include "services/console_layout.hpp"
#include "services/messages.hpp"
#include "util/fs.hpp"
#include "util/log.hpp"
#include "util/thread.hpp"

namespace openpsapp::tile {
namespace {
std::mutex g_mu;
std::string g_state = "idle";
std::string g_error;

void Set(std::string state, std::string error = "") {
    std::lock_guard<std::mutex> lock(g_mu);
    g_state = std::move(state);
    g_error = std::move(error);
}

bool Installed() { return fs::StatPath(platform::ConsolePath(layout::AppMetaDir(kTitleId))).has_value(); }
}

Status GetStatus() {
    Status s;
    s.embedded = kPkgSize > 0;
    s.installed = Installed();
    std::lock_guard<std::mutex> lock(g_mu);
    s.state = s.embedded ? g_state : "not embedded";
    s.error = g_error;
    return s;
}

void InstallIfNeeded(const std::string &data_dir, int port, const std::string &language) {
    if (port != kDeeplinkPort) log::Error("tile: it opens port %d but the payload listens on %d", kDeeplinkPort, port);
    if (kPkgSize == 0) {
        log::Info("tile: no PKG embedded in this build");
        return;
    }
    if (Installed()) {
        Set("installed");
        return;
    }
    Set("installing");
    Spawn([data_dir, port, language]() {
        std::string path = data_dir + "/tile.pkg";
        std::string err;
        if (!fs::WriteFileAtomic(path, std::string_view(reinterpret_cast<const char *>(kPkg), kPkgSize))) {
            err = "could not write " + path;
        } else if (platform::InstallPkg(path, &err)) {
            unlink(path.c_str());
            Set("installed");
            log::Info("tile: installed %s", kTitleId);
            platform::Notify(messages::Text(messages::Id::TileInstalled, language, port));
            return;
        }
        Set("failed", err);
        log::Error("tile: install failed: %s", err.c_str());
    });
}

}
