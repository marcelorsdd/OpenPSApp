#include <sys/stat.h>

#include <cstdio>

#include "cloud/asset_hash.hpp"
#include "cloud/client.hpp"
#include "cloud/controller.hpp"
#include "cloud/snapshot.hpp"
#include "cloud/store.hpp"
#include "fixture.hpp"
#include "helpers.hpp"
#include "platform/platform.hpp"
#include "services/hidden.hpp"
#include "services/library.hpp"
#include "test.hpp"
#include "util/fs.hpp"
#include "util/sha256.hpp"

using namespace openpsapp;
using namespace openpsapp::cloud;

namespace {
std::string TempPath(const char *name) {
    std::string dir = platform::Root() + "/data/cloud-test";
    fs::EnsureDir(platform::Root() + "/data");
    fs::EnsureDir(dir);
    std::string path = dir + "/" + name;
    std::remove(path.c_str());
    return path;
}
}

TEST(cloud_store_keeps_links_per_user_privately) {
    std::string path = TempPath("cloud.json");
    {
        CloudStore store(path);
        CHECK(!store.Get("1eb71bbd").has_value());
        CHECK(store.Put("1eb71bbd", {"opsa_token_a", "device-a", "Marcelo"}));
        CHECK(store.Put("2aa00001", {"opsa_token_b", "device-b", "Guest"}));
    }
    CloudStore reloaded(path);
    auto a = reloaded.Get("1eb71bbd");
    CHECK(a.has_value());
    if (a) {
        CHECK_EQ(a->token, std::string("opsa_token_a"));
        CHECK_EQ(a->device_id, std::string("device-a"));
        CHECK_EQ(a->profile_name, std::string("Marcelo"));
    }
    CHECK(reloaded.Remove("2aa00001"));
    CHECK(!CloudStore(path).Get("2aa00001").has_value());
    CHECK(CloudStore(path).Get("1eb71bbd").has_value());
    struct stat st;
    CHECK(stat(path.c_str(), &st) == 0);
    CHECK_EQ(int(st.st_mode & 0777), 0600);
}

TEST(asset_hashes_match_content_and_are_cached) {
    std::string image = TempPath("image.png");
    std::string cache = TempPath("asset-hashes.json");
    std::string bytes = "\x89PNG\r\n\x1a\nnot-really-a-png";
    CHECK(fs::WriteFileAtomic(image, bytes));
    icons::Location where{image, 0, -1, icons::FileVersion(image)};
    std::string expected = Sha256::Hex(bytes);
    {
        AssetHasher hasher(cache);
        CHECK_EQ(hasher.Hash(where), expected);
        hasher.Flush();
    }
    std::remove(image.c_str());
    AssetHasher cached(cache);
    CHECK_EQ(cached.Hash(where), expected);
    CHECK_EQ(cached.Hash({image + ".missing", 0, -1, "1-1"}), std::string(""));
}

TEST(asset_hashes_cover_byte_ranges) {
    std::string file = TempPath("container.bin");
    CHECK(fs::WriteFileAtomic(file, "headerPAYLOADtrailer"));
    AssetHasher hasher(TempPath("asset-hashes-range.json"));
    CHECK_EQ(hasher.Hash({file, 6, 7, icons::FileVersion(file)}), Sha256::Hex("PAYLOAD"));
}

TEST(snapshot_describes_games_sets_and_images_per_the_contract) {
    std::string data_dir = platform::Root() + "/data/openpsapp";
    fs::EnsureDir(data_dir);
    hidden::Load(data_dir);
    setenv("OPENPSAPP_USER", fixture::kUserHex, 1);
    auto lib = CurrentLibrary("pt-BR");
    AssetHasher hasher(TempPath("asset-hashes-snapshot.json"));
    auto hash = [&](const icons::Location &where) { return hasher.Hash(where); };

    Snapshot snap = BuildSnapshot(*lib, "Marcelo", "pt-BR", hash);
    std::string icon_bytes;
    CHECK(fs::ReadFile(platform::Root() + "/user/appmeta/PPSA01234/icon0.png", &icon_bytes));
    std::string icon_id = Sha256::Hex(icon_bytes);

    CHECK(snap.manifest.find("\"profile\":{\"name\":\"Marcelo\",\"language\":\"pt-BR\"}") != std::string::npos);
    CHECK(snap.manifest.find("\"titleId\":\"PPSA01234\"") != std::string::npos);
    CHECK(snap.manifest.find("\"iconId\":\"" + icon_id + "\"") != std::string::npos);
    CHECK(snap.manifest.find("\"trophySetId\":\"NPWR00001_00\"") != std::string::npos);
    CHECK(snap.manifest.find("\"titleId\":\"PPSA05555\",\"name\":\"Browse Name\",\"playtimeSeconds\":60,\"lastPlayedAt\":null,\"iconId\":null") !=
          std::string::npos);
    CHECK(snap.assets.count(icon_id) == 1);

    const TrophySet *set = lib->FindSet(fixture::kSetId);
    CHECK(set != nullptr);
    if (set) CHECK(snap.manifest.find("{\"id\":\"NPWR00001_00\",\"version\":\"" + set->version + "\"}") != std::string::npos);
    CHECK_EQ(snap.sets.size(), size_t(1));
    std::string set_json = snap.sets[fixture::kSetId];
    CHECK(set_json.find("\"name\":\"Astro Teste\"") != std::string::npos);
    CHECK(set_json.find("\"earned\":true,\"earnedAt\":\"2025-11-07T20:00:00Z\"") != std::string::npos);
    CHECK(set_json.find("\"grade\":\"platinum\"") != std::string::npos);

    Snapshot same = BuildSnapshot(*lib, "Marcelo", "pt-BR", hash);
    CHECK_EQ(same.fingerprint, snap.fingerprint);
    CHECK(hidden::Set("PPSA05555", true));
    Snapshot hidden_one = BuildSnapshot(*lib, "Marcelo", "pt-BR", hash);
    CHECK(hidden_one.manifest.find("\"titleId\":\"PPSA05555\"") != std::string::npos);
    CHECK(hidden_one.manifest.find("\"trophySetId\":\"NPWR00002_00\",\"hidden\":true") != std::string::npos);
    CHECK(hidden_one.fingerprint != snap.fingerprint);
    CHECK(hidden::Set("PPSA05555", false));
    unsetenv("OPENPSAPP_USER");
}

namespace {
struct FakeServer {
    std::vector<net::HttpsRequest> seen;
    std::function<net::HttpsResponse(const net::HttpsRequest &)> handler;

    Transport transport() {
        return [this](const net::HttpsRequest &r) {
            seen.push_back(r);
            return handler(r);
        };
    }
};

net::HttpsResponse Reply(int status, std::string body = "") {
    net::HttpsResponse r;
    r.status = status;
    r.body = std::move(body);
    return r;
}

std::string Header(const net::HttpsRequest &r, const std::string &name) {
    for (const auto &[k, v] : r.headers) if (k == name) return v;
    return "";
}
}

TEST(cloud_client_requests_a_pairing_code) {
    FakeServer server;
    server.handler = [](const net::HttpsRequest &) {
        return Reply(200, R"({"deviceCode":"dc","userCode":"K7Q4-PX2A","verificationUri":"https://openpsapp.test/link","expiresIn":600,"interval":5})");
    };
    CloudClient client("https://api.openpsapp.test/", server.transport());
    std::string err;
    auto code = client.RequestCode("Mar\"celo", &err);
    CHECK(code.has_value());
    if (code) {
        CHECK_EQ(code->device_code, std::string("dc"));
        CHECK_EQ(code->user_code, std::string("K7Q4-PX2A"));
        CHECK_EQ(code->verification_uri, std::string("https://openpsapp.test/link"));
        CHECK_EQ(code->expires_in, 600);
        CHECK_EQ(code->interval, 5);
    }
    CHECK_EQ(server.seen.size(), size_t(1));
    CHECK_EQ(server.seen[0].method, std::string("POST"));
    CHECK_EQ(server.seen[0].url, std::string("https://api.openpsapp.test/api/v1/devices/codes"));
    CHECK_EQ(server.seen[0].body, std::string(R"({"profileName":"Mar\"celo"})"));
    CHECK_EQ(Header(server.seen[0], "Content-Type"), std::string("application/json"));

    server.handler = [](const net::HttpsRequest &) { return Reply(429, R"({"error":"rate_limited"})"); };
    CHECK(!client.RequestCode("Marcelo", &err).has_value());
    CHECK(!err.empty());
}

TEST(cloud_client_maps_token_polling_answers) {
    FakeServer server;
    CloudClient client("https://api.openpsapp.test", server.transport());
    server.handler = [](const net::HttpsRequest &) { return Reply(428, R"({"error":"authorization_pending"})"); };
    CHECK(client.PollToken("dc", "Marcelo").state == TokenState::Pending);
    server.handler = [](const net::HttpsRequest &) { return Reply(410); };
    CHECK(client.PollToken("dc", "Marcelo").state == TokenState::Expired);
    server.handler = [](const net::HttpsRequest &) { return Reply(404); };
    CHECK(client.PollToken("dc", "Marcelo").state == TokenState::Invalid);
    server.handler = [](const net::HttpsRequest &) {
        net::HttpsResponse r;
        r.error = "timeout: connect";
        return r;
    };
    TokenResult failed = client.PollToken("dc", "Marcelo");
    CHECK(failed.state == TokenState::Failed);
    CHECK_EQ(failed.error, std::string("timeout: connect"));
    server.handler = [](const net::HttpsRequest &) { return Reply(200, R"({"deviceId":"dev-1","token":"opsa_abc"})"); };
    TokenResult granted = client.PollToken("dc", "Marcelo");
    CHECK(granted.state == TokenState::Granted);
    CHECK_EQ(granted.link.token, std::string("opsa_abc"));
    CHECK_EQ(granted.link.device_id, std::string("dev-1"));
    CHECK_EQ(granted.link.profile_name, std::string("Marcelo"));
    CHECK_EQ(server.seen.back().body, std::string(R"({"deviceCode":"dc"})"));
}

TEST(cloud_client_uploads_only_what_the_server_asks_for) {
    std::string image = TempPath("upload.png");
    CHECK(fs::WriteFileAtomic(image, "image-bytes"));
    std::string asset_id(64, 'a');
    Snapshot snap;
    snap.manifest = R"({"m":1})";
    snap.sets = {{"NPWR00001_00", R"({"s":1})"}, {"NPWR00002_00", R"({"s":2})"}};
    snap.assets = {{asset_id, {image, 0, -1, icons::FileVersion(image)}}, {std::string(64, 'b'), {image, 0, -1, "x"}}};

    FakeServer server;
    server.handler = [&](const net::HttpsRequest &r) {
        if (r.url.find("/sync/manifest") != std::string::npos)
            return Reply(200, R"({"needSets":["NPWR00002_00"],"needAssets":[")" + asset_id + R"("]})");
        if (r.url.find("/sync/sets/") != std::string::npos) return Reply(204);
        return Reply(201);
    };
    CloudClient client("https://api.openpsapp.test", server.transport());
    auto read = [](const icons::Location &where, std::string *out) { return fs::ReadFile(where.path, out); };
    SyncResult result = client.Sync("opsa_tok", snap, read);
    CHECK(result.ok);
    CHECK_EQ(result.sets, 1);
    CHECK_EQ(result.assets, 1);
    CHECK_EQ(server.seen.size(), size_t(3));
    if (server.seen.size() == 3) {
        CHECK_EQ(server.seen[0].url, std::string("https://api.openpsapp.test/api/v1/sync/manifest"));
        CHECK_EQ(server.seen[0].body, std::string(R"({"m":1})"));
        for (const auto &r : server.seen) CHECK_EQ(Header(r, "Authorization"), std::string("Bearer opsa_tok"));
        CHECK_EQ(server.seen[1].method, std::string("PUT"));
        CHECK_EQ(server.seen[1].url, std::string("https://api.openpsapp.test/api/v1/sync/sets/NPWR00002_00"));
        CHECK_EQ(server.seen[1].body, std::string(R"({"s":2})"));
        CHECK_EQ(server.seen[2].url, "https://api.openpsapp.test/api/v1/sync/assets/" + asset_id);
        CHECK_EQ(Header(server.seen[2], "Content-Type"), std::string("application/octet-stream"));
        CHECK_EQ(server.seen[2].body, std::string("image-bytes"));
    }
}

TEST(cloud_client_reports_revoked_tokens_and_failures) {
    Snapshot snap;
    snap.manifest = "{}";
    snap.sets = {{"NPWR00001_00", "{}"}};
    FakeServer server;
    CloudClient client("https://api.openpsapp.test", server.transport());
    auto read = [](const icons::Location &, std::string *) { return false; };
    server.handler = [](const net::HttpsRequest &) { return Reply(401, R"({"error":"unauthorized"})"); };
    SyncResult revoked = client.Sync("opsa_tok", snap, read);
    CHECK(!revoked.ok);
    CHECK(revoked.unauthorized);
    server.handler = [](const net::HttpsRequest &r) {
        if (r.url.find("/sync/manifest") != std::string::npos) return Reply(200, R"({"needSets":["NPWR00001_00"],"needAssets":[]})");
        return Reply(500, R"({"error":"internal"})");
    };
    SyncResult failed = client.Sync("opsa_tok", snap, read);
    CHECK(!failed.ok);
    CHECK(!failed.unauthorized);
    CHECK(!failed.error.empty());
}

namespace {
constexpr int64_t kMinute = 60 * 1000;

struct ControllerRig {
    FakeServer server;
    CloudClient client{"https://api.openpsapp.test", server.transport()};
    CloudStore store;
    std::string fingerprint = "f1";
    int manifests = 0;
    int token_polls = 0;
    int manifest_status = 200;
    int token_status = 428;
    int expires_in = 600;
    CloudController controller;

    explicit ControllerRig(const char *store_file)
        : store(TempPath(store_file)),
          controller(&client, &store,
                     SnapshotSource{[] { return std::optional<std::pair<std::string, std::string>>({"1eb71bbd", "Marcelo"}); },
                                    [this] {
                                        Snapshot snap;
                                        snap.manifest = "{}";
                                        snap.fingerprint = fingerprint;
                                        return snap;
                                    },
                                    [](const icons::Location &, std::string *) { return false; }}) {
        server.handler = [this](const net::HttpsRequest &r) {
            if (r.url.find("/devices/codes") != std::string::npos)
                return Reply(200, R"({"deviceCode":"dc","userCode":"K7Q4-PX2A","verificationUri":"https://openpsapp.test/link","expiresIn":)" +
                                      std::to_string(expires_in) + R"(,"interval":5})");
            if (r.url.find("/devices/token") != std::string::npos) {
                token_polls++;
                return token_status == 200 ? Reply(200, R"({"deviceId":"dev-1","token":"opsa_tok"})") : Reply(token_status);
            }
            if (r.url.find("/sync/manifest") != std::string::npos) {
                manifests++;
                return manifest_status == 200 ? Reply(200, R"({"needSets":[],"needAssets":[]})") : Reply(manifest_status);
            }
            return Reply(404);
        };
    }

    void Link(int64_t now) {
        std::string err;
        CHECK(controller.StartLink(now, &err));
        token_status = 200;
        controller.Tick(now + 5000);
    }
};
}

TEST(cloud_controller_pairs_then_syncs) {
    ControllerRig rig("controller-pair.json");
    std::string err;
    CHECK(rig.controller.StartLink(0, &err));
    CloudStatus pairing = rig.controller.Status();
    CHECK(pairing.pairing.has_value());
    if (pairing.pairing) CHECK_EQ(pairing.pairing->user_code, std::string("K7Q4-PX2A"));
    CHECK(!pairing.linked);

    rig.controller.Tick(1000);
    CHECK_EQ(rig.token_polls, 0);
    rig.controller.Tick(5000);
    CHECK_EQ(rig.token_polls, 1);
    CHECK(rig.controller.Status().pairing.has_value());

    rig.token_status = 200;
    rig.controller.Tick(10000);
    CloudStatus linked = rig.controller.Status();
    CHECK(linked.linked);
    CHECK(!linked.pairing.has_value());
    CHECK_EQ(linked.profile_name, std::string("Marcelo"));
    auto stored = rig.store.Get("1eb71bbd");
    CHECK(stored.has_value() && stored->token == "opsa_tok");

    rig.controller.Tick(10001);
    CHECK_EQ(rig.manifests, 1);
    CHECK(!rig.controller.Status().last_sync_at.empty());
}

TEST(cloud_controller_syncs_on_changes_and_every_fifteen_minutes) {
    ControllerRig rig("controller-schedule.json");
    rig.Link(0);
    rig.controller.Tick(6000);
    CHECK_EQ(rig.manifests, 1);
    rig.controller.Tick(6000 + 30 * 1000);
    CHECK_EQ(rig.manifests, 1);
    rig.controller.Tick(6000 + kMinute + 1000);
    CHECK_EQ(rig.manifests, 1);
    rig.fingerprint = "f2";
    rig.controller.Tick(6000 + 2 * kMinute + 2000);
    CHECK_EQ(rig.manifests, 2);
    rig.controller.Tick(6000 + 2 * kMinute + 2000 + 15 * kMinute);
    CHECK_EQ(rig.manifests, 3);
    rig.controller.RequestSync();
    rig.controller.Tick(6000 + 2 * kMinute + 4000 + 15 * kMinute);
    CHECK_EQ(rig.manifests, 4);
}

TEST(cloud_controller_unlinks_when_the_website_revokes_the_console) {
    ControllerRig rig("controller-revoke.json");
    rig.Link(0);
    rig.manifest_status = 401;
    rig.controller.Tick(6000);
    CloudStatus status = rig.controller.Status();
    CHECK(!status.linked);
    CHECK(!status.last_error.empty());
    CHECK(!rig.store.Get("1eb71bbd").has_value());
}

TEST(cloud_controller_backs_off_after_a_failed_sync) {
    ControllerRig rig("controller-backoff.json");
    rig.Link(0);
    rig.manifest_status = 500;
    rig.controller.Tick(6000);
    CHECK_EQ(rig.manifests, 1);
    CHECK(!rig.controller.Status().last_error.empty());
    rig.controller.Tick(8000);
    CHECK_EQ(rig.manifests, 1);
    rig.manifest_status = 200;
    rig.controller.Tick(6000 + kMinute + 1000);
    CHECK_EQ(rig.manifests, 2);
    CHECK(rig.controller.Status().last_error.empty());
}

TEST(cloud_controller_drops_an_expired_code_and_can_unlink) {
    ControllerRig rig("controller-expire.json");
    rig.expires_in = 10;
    std::string err;
    CHECK(rig.controller.StartLink(0, &err));
    rig.controller.Tick(11000);
    CloudStatus status = rig.controller.Status();
    CHECK(!status.pairing.has_value());
    CHECK(!status.last_error.empty());

    rig.expires_in = 600;
    rig.Link(20000);
    CHECK(rig.controller.Status().linked);
    rig.controller.Unlink();
    CHECK(!rig.controller.Status().linked);
    CHECK(!rig.store.Get("1eb71bbd").has_value());
}

TEST(cloud_routes_report_an_unconfigured_cloud) {
    test::App app;
    http::Response status = test::Get(app.router, "/api/v1/cloud");
    CHECK_EQ(status.status, 200);
    CHECK(status.body.find("\"configured\":false") != std::string::npos);
    CHECK_EQ(test::Call(app.router, "POST", "/api/v1/cloud/link").status, 409);
}
