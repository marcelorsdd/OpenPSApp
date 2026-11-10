#include "http/client.hpp"
#include "routes/routes.hpp"
#include "util/json.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::routes {
namespace {
constexpr const char *kMarker = "\"source\":\"console\"";
}

void RegisterHealth(http::Router &router, const Context &ctx) {
    router.Get("/api/v1/health", [&ctx](const http::Request &) {
        JsonWriter w;
        w.BeginObject()
            .Key("ok").Bool(true)
            .Key("version").String(ctx.version)
            .Key("source").String("console")
            .Key("capturedAt").String(timeutil::NowIso())
            .Key("language").String(ctx.language)
            .EndObject();
        return http::Json(w.take());
    });
}

bool ServedByOpenPSApp(uint16_t port) {
    std::string body;
    return http::LocalGet(port, "/api/v1/health", &body) && body.find(kMarker) != std::string::npos;
}

}
