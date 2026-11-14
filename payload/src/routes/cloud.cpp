#include "cloud/service.hpp"
#include "routes/routes.hpp"
#include "util/json.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::routes {
namespace {
http::Response StatusJson(const cloud::CloudController *controller) {
    JsonWriter w;
    if (!controller) {
        w.BeginObject().Key("configured").Bool(false).Key("linked").Bool(false).EndObject();
        return http::Json(w.take());
    }
    cloud::CloudStatus s = controller->Status();
    w.BeginObject()
        .Key("configured").Bool(true)
        .Key("linked").Bool(s.linked)
        .Key("syncing").Bool(s.syncing)
        .Key("profileName").StringOrNull(s.profile_name)
        .Key("lastSyncAt").StringOrNull(s.last_sync_at)
        .Key("lastError").StringOrNull(s.last_error)
        .Key("pairing");
    if (s.pairing) {
        int64_t left = (s.pairing->expires_at_ms - timeutil::NowMillis()) / 1000;
        w.BeginObject()
            .Key("userCode").String(s.pairing->user_code)
            .Key("verificationUri").String(s.pairing->verification_uri)
            .Key("expiresIn").Int(left > 0 ? left : 0)
            .EndObject();
    } else {
        w.Null();
    }
    w.EndObject();
    return http::Json(w.take());
}

http::Response NotConfigured() { return http::Error(409, "cloud not configured"); }
}

void RegisterCloud(http::Router &router, const Context &) {
    router.Get("/api/v1/cloud", [](const http::Request &) { return StatusJson(cloud::Controller()); });

    router.Post("/api/v1/cloud/link", [](const http::Request &) {
        cloud::CloudController *controller = cloud::Controller();
        if (!controller) return NotConfigured();
        std::string err;
        if (!controller->StartLink(timeutil::NowMillis(), &err)) return http::Error(502, err);
        return StatusJson(controller);
    });

    router.Post("/api/v1/cloud/unlink", [](const http::Request &) {
        cloud::CloudController *controller = cloud::Controller();
        if (!controller) return NotConfigured();
        controller->Unlink();
        return StatusJson(controller);
    });

    router.Post("/api/v1/cloud/sync", [](const http::Request &) {
        cloud::CloudController *controller = cloud::Controller();
        if (!controller) return NotConfigured();
        controller->RequestSync();
        return StatusJson(controller);
    });
}

}
