#include "routes/web.hpp"

#include "routes/routes.hpp"

namespace openpsapp::routes {
namespace {
const web::Asset *Find(const std::string &path) {
    for (unsigned i = 0; i < web::kAssetCount; ++i) {
        if (path == web::kAssets[i].path) return &web::kAssets[i];
    }
    return nullptr;
}

http::Response Serve(const web::Asset &asset, const char *cache_control) {
    http::Response res;
    res.content_type = asset.content_type;
    res.body.assign(reinterpret_cast<const char *>(asset.data), asset.size);
    if (asset.gzip) res.content_encoding = "gzip";
    res.cache_control = cache_control;
    return res;
}
}

void RegisterWeb(http::Router &router, const Context &) {
    router.Get("/", [](const http::Request &) {
        const web::Asset *page = Find("/index.html");
        return page ? Serve(*page, "no-cache") : http::Error(404, "UI not embedded");
    });
    router.Get("/:file", [](const http::Request &req) {
        const web::Asset *asset = Find("/" + req.params.at("file"));
        return asset ? Serve(*asset, "no-cache") : http::Error(404, "not found");
    });
    router.Get("/assets/:file", [](const http::Request &req) {
        const web::Asset *asset = Find("/assets/" + req.params.at("file"));
        return asset ? Serve(*asset, http::kCacheForever) : http::Error(404, "not found");
    });
}

}
