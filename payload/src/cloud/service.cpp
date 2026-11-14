#include "cloud/service.hpp"

#include <unistd.h>

#include <memory>

#include "cloud/asset_hash.hpp"
#include "net/ca_roots.hpp"
#include "platform/platform.hpp"
#include "services/library.hpp"
#include "util/fs.hpp"
#include "util/log.hpp"
#include "util/thread.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::cloud {
namespace {
constexpr int64_t kMaxAssetBytes = 32 << 20;
constexpr unsigned kTickSeconds = 2;

std::unique_ptr<CloudStore> g_store;
std::unique_ptr<AssetHasher> g_hasher;
std::unique_ptr<CloudClient> g_client;
std::unique_ptr<CloudController> g_controller;
}

CloudController *Controller() { return g_controller.get(); }

void Start(const routes::Context &ctx) {
    if (ctx.config.server.empty() || g_controller) return;
    g_store = std::make_unique<CloudStore>(ctx.data_dir + "/cloud.json");
    g_hasher = std::make_unique<AssetHasher>(ctx.data_dir + "/asset-hashes.json");
    g_client = std::make_unique<CloudClient>(ctx.config.server, [](const net::HttpsRequest &request) {
        return net::HttpsSend(request, net::ProductionRoots());
    });
    std::string language = ctx.language;
    SnapshotSource source{
        []() -> std::optional<std::pair<std::string, std::string>> {
            auto user = platform::ForegroundUser();
            if (!user) return std::nullopt;
            return std::make_pair(user->id_hex, user->name);
        },
        [language] {
            auto user = platform::ForegroundUser();
            auto lib = CurrentLibrary(language);
            Snapshot snapshot = BuildSnapshot(*lib, user ? user->name : "", language,
                                              [](const icons::Location &where) { return g_hasher->Hash(where); });
            g_hasher->Flush();
            return snapshot;
        },
        [](const icons::Location &where, std::string *out) {
            return fs::ReadRange(where.path, where.offset, where.length, out, kMaxAssetBytes);
        },
    };
    g_controller = std::make_unique<CloudController>(g_client.get(), g_store.get(), std::move(source));
    Spawn([] {
        for (;;) {
            g_controller->Tick(timeutil::NowMillis());
            sleep(kTickSeconds);
        }
    });
    log::Info("cloud sync enabled for %s", ctx.config.server.c_str());
}

}
