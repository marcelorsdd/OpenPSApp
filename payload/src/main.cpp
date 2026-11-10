#include <unistd.h>

#include <csignal>
#include <string>

#include "config.hpp"
#include "http/server.hpp"
#include "platform/platform.hpp"
#include "routes/routes.hpp"
#include "services/hidden.hpp"
#include "services/messages.hpp"
#include "services/tile.hpp"
#include "util/crash.hpp"
#include "util/fs.hpp"
#include "util/log.hpp"
#include "util/stats.hpp"
#include "util/timeutil.hpp"
#include "version.hpp"


using namespace openpsapp;

int main() {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    stats::Get().started_at = timeutil::NowUnix();

    std::string err;
    bool escalated = platform::Escalate(&err);
    platform::SetProcessName("openpsapp.elf");

    std::string data_dir = platform::ConsolePath("/data/openpsapp");
    fs::EnsureDir(data_dir);
    log::Init(data_dir + "/openpsapp.log");
    crash::InstallHandlers();
    log::Info("OpenPSApp %s starting on %s (pid %d)", OPENPSAPP_VERSION, platform::Name(), getpid());
    if (!escalated) log::Error("privilege escalation failed: %s", err.c_str());

    routes::Context ctx;
    ctx.version = OPENPSAPP_VERSION;
    ctx.data_dir = data_dir;
    std::string warning;
    ctx.config = LoadOrCreateConfig(data_dir + "/config.json", &warning);
    if (!warning.empty()) log::Error("config: %s", warning.c_str());
    ctx.language = ResolveLanguage(ctx.config.language);
    hidden::Load(data_dir);

    http::Router router;
    routes::RegisterAll(router, ctx);

    http::Server server(router);
    uint16_t port = static_cast<uint16_t>(ctx.config.port);
    if (!server.Listen(port, &err)) {
        if (routes::ServedByOpenPSApp(port)) {
            log::Info("another OpenPSApp already serves :%d; exiting", ctx.config.port);
            platform::Notify(messages::Text(messages::Id::AlreadyRunning, ctx.language, ctx.config.port));
            return 0;
        }
        log::Error("listen failed: %s", err.c_str());
        platform::Notify(messages::Text(messages::Id::PortBusy, ctx.language, ctx.config.port));
        return 1;
    }
    log::Info("listening on :%d, language %s", ctx.config.port, ctx.language.c_str());
    platform::Notify(messages::Text(messages::Id::Started, ctx.language, ctx.config.port));
    tile::InstallIfNeeded(data_dir, ctx.config.port, ctx.language);

    server.Serve();
    return 0;
}
