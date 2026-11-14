#include "routes/cloud.hpp"

#include "cloud/service.hpp"
#include "routes/routes.hpp"
#include "util/json.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::routes {
namespace {
using ControllerFn = std::function<cloud::CloudController *()>;

http::Response StatusJson(const cloud::CloudController *controller, bool local) {
    JsonWriter w;
    if (!controller) {
        w.BeginObject().Bool("configured", false).Bool("linked", false).Bool("local", local).EndObject();
        return http::Json(w.take());
    }
    cloud::CloudStatus s = controller->Status();
    w.BeginObject()
        .Bool("configured", true)
        .Bool("local", local)
        .Bool("linked", s.linked)
        .Bool("syncing", s.syncing)
        .StringOrNull("profileName", s.profile_name)
        .StringOrNull("lastSyncAt", s.last_sync_at)
        .StringOrNull("lastError", s.last_error)
        .Key("pairing");
    if (s.pairing) {
        int64_t left = (s.pairing->expires_at_ms - timeutil::NowMillis()) / 1000;
        w.BeginObject();
        if (local) w.String("userCode", s.pairing->user_code).String("verificationUri", s.pairing->verification_uri);
        w.Int("expiresIn", left > 0 ? left : 0).EndObject();
    } else {
        w.Null();
    }
    w.EndObject();
    return http::Json(w.take());
}

void PostConsoleAction(http::Router &router, std::string path, int port, ControllerFn controller,
                       std::function<http::Response(cloud::CloudController &)> run) {
    router.Post(std::move(path), [port, controller, run](const http::Request &req) {
        if (!FromConsole(req, port) || !FromApp(req)) return http::Error(403, "console_only");
        cloud::CloudController *c = controller();
        if (!c) return http::Error(409, "not_configured");
        return run(*c);
    });
}
}

bool FromConsole(const http::Request &req, int port) {
    if (!http::IsLoopback(req.peer)) return false;
    std::string host = http::Header(req, "host");
    std::string suffix = ":" + std::to_string(port);
    return host == "127.0.0.1" + suffix || host == "localhost" + suffix;
}

void RegisterCloud(http::Router &router, int port, ControllerFn controller) {
    router.Get("/api/v1/cloud", [port, controller](const http::Request &req) {
        return StatusJson(controller(), FromConsole(req, port));
    });

    PostConsoleAction(router, "/api/v1/cloud/link", port, controller, [](cloud::CloudController &c) {
        std::string err;
        if (!c.StartLink(timeutil::NowMillis(), &err)) return http::Error(err == "cloud_unavailable" ? 502 : 409, err);
        return StatusJson(&c, true);
    });

    PostConsoleAction(router, "/api/v1/cloud/unlink", port, controller, [](cloud::CloudController &c) {
        c.Unlink();
        return StatusJson(&c, true);
    });

    PostConsoleAction(router, "/api/v1/cloud/sync", port, controller, [](cloud::CloudController &c) {
        c.RequestSync();
        return StatusJson(&c, true);
    });
}

void RegisterCloud(http::Router &router, const Context &ctx) {
    RegisterCloud(router, ctx.config.port, cloud::Controller);
}

}
