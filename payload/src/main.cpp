#include <unistd.h>

#include <csignal>
#include <string>

#include "config.hpp"
#include "http/server.hpp"
#include "platform/platform.hpp"
#include "routes/routes.hpp"
#include "services/hidden.hpp"
#include "services/tile.hpp"
#include "util/crash.hpp"
#include "util/fs.hpp"
#include "util/log.hpp"
#include "util/stats.hpp"
#include "util/timeutil.hpp"

#ifndef PLAYTRACK_VERSION
#define PLAYTRACK_VERSION "dev"
#endif

using namespace playtrack;

int main() {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    stats::Get().started_at = timeutil::NowUnix();

    std::string err;
    bool escalated = platform::Escalate(&err);
    platform::SetProcessName("playtrack.elf");

    std::string data_dir = platform::ConsolePath("/data/playtrack");
    fs::EnsureDir(data_dir);
    log::Init(data_dir + "/playtrack.log");
    crash::InstallHandlers();
    log::Info("PlayTrack %s starting on %s (pid %d)", PLAYTRACK_VERSION, platform::Name(), getpid());
    if (!escalated) log::Error("privilege escalation failed: %s", err.c_str());

    routes::Context ctx;
    ctx.version = PLAYTRACK_VERSION;
    ctx.data_dir = data_dir;
    std::string warning;
    ctx.config = LoadOrCreateConfig(data_dir + "/config.json", &warning);
    if (!warning.empty()) log::Error("config: %s", warning.c_str());
    hidden::Load(data_dir);

    http::Router router;
    routes::RegisterAll(router, ctx);

    http::Server server(router);
    uint16_t port = static_cast<uint16_t>(ctx.config.port);
    if (!server.Listen(port, &err)) {
        if (routes::ServedByPlayTrack(port)) {
            log::Info("another PlayTrack already serves :%d; exiting", ctx.config.port);
            platform::Notify("PlayTrack já está rodando");
            return 0;
        }
        log::Error("listen failed: %s", err.c_str());
        platform::Notify("PlayTrack: a porta " + std::to_string(ctx.config.port) + " está em uso por outro app");
        return 1;
    }
    log::Info("listening on :%d", ctx.config.port);
    platform::Notify("PlayTrack ativo na porta " + std::to_string(ctx.config.port));
    tile::InstallIfNeeded(data_dir, ctx.config.port);

    server.Serve();
    return 0;
}
