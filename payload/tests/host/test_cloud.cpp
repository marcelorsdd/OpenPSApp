#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>

#include "cloud/asset_hash.hpp"
#include "cloud/client.hpp"
#include "cloud/controller.hpp"
#include "cloud/snapshot.hpp"
#include "cloud/store.hpp"
#include "fixture.hpp"
#include "helpers.hpp"
#include "platform/platform.hpp"
#include "routes/cloud.hpp"
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
        CHECK(store.Put("1eb71bbd", {"opsa_token_a", "device-a", "tester"}));
        CHECK(store.Put("2aa00001", {"opsa_token_b", "device-b", "Guest"}));
    }
    CloudStore reloaded(path);
    auto a = reloaded.Get("1eb71bbd");
    CHECK(a.has_value());
    if (a) {
        CHECK_EQ(a->token, std::string("opsa_token_a"));
        CHECK_EQ(a->device_id, std::string("device-a"));
        CHECK_EQ(a->profile_name, std::string("tester"));
    }
    CHECK(reloaded.Remove("2aa00001"));
    CHECK(!CloudStore(path).Get("2aa00001").has_value());
    CHECK(CloudStore(path).Get("1eb71bbd").has_value());
    struct stat st;
    CHECK(stat(path.c_str(), &st) == 0);
    CHECK_EQ(int(st.st_mode & 0777), 0600);
}

TEST(cloud_store_is_private_even_over_a_stale_temp_file) {
    std::string path = TempPath("cloud-stale.json");
    CHECK(fs::WriteFileAtomic(path, "{}"));
    CHECK(fs::WriteFileAtomic(path + ".tmp", "stale"));
    struct stat st;
    CHECK(stat((path + ".tmp").c_str(), &st) == 0);
    CHECK_EQ(int(st.st_mode & 0777), 0644);
    CloudStore store(path);
    CHECK(store.Put("1eb71bbd", {"opsa_token_a", "device-a", "tester"}));
    CHECK(stat(path.c_str(), &st) == 0);
    CHECK_EQ(int(st.st_mode & 0777), 0600);
    CHECK(stat((path + ".tmp").c_str(), &st) != 0);
}

TEST(cloud_store_remembers_tokens_to_revoke) {
    std::string path = TempPath("cloud-revoke.json");
    {
        CloudStore store(path);
        CHECK(!store.Retire("1eb71bbd").has_value());
        CHECK(store.Revocations().empty());
        CHECK(store.Put("1eb71bbd", {"opsa_old", "device-a", "tester"}));
        auto token = store.Retire("1eb71bbd");
        CHECK(token.has_value() && *token == "opsa_old");
        CHECK(!store.Get("1eb71bbd").has_value());
    }
    CloudStore reloaded(path);
    CHECK(reloaded.Revocations() == std::vector<std::string>{"opsa_old"});
    CHECK(!reloaded.Get("1eb71bbd").has_value());
    struct stat st;
    CHECK(stat(path.c_str(), &st) == 0);
    CHECK_EQ(int(st.st_mode & 0777), 0600);
    CHECK(reloaded.Revoked("opsa_old"));
    CHECK(reloaded.Revoked("opsa_unknown"));
    CHECK(CloudStore(path).Revocations().empty());
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

    Snapshot snap = BuildSnapshot(*lib, "tester", "pt-BR", hash);
    std::string icon_bytes;
    CHECK(fs::ReadFile(platform::Root() + "/user/appmeta/PPSA01234/icon0.png", &icon_bytes));
    std::string icon_id = Sha256::Hex(icon_bytes);

    CHECK(snap.manifest.find("\"profile\":{\"name\":\"tester\",\"language\":\"pt-BR\"}") != std::string::npos);
    CHECK(snap.manifest.find("\"titleId\":\"PPSA01234\"") != std::string::npos);
    CHECK(snap.manifest.find("\"iconId\":\"" + icon_id + "\"") != std::string::npos);
    CHECK(snap.manifest.find("\"trophySetId\":\"NPWR00001_00\"") != std::string::npos);
    CHECK(snap.manifest.find("\"titleId\":\"PPSA05555\",\"name\":\"Browse "
                             "Name\",\"playtimeSeconds\":60,\"lastPlayedAt\":null,\"iconId\":null") !=
          std::string::npos);
    CHECK(snap.assets.count(icon_id) == 1);

    const TrophySet *set = lib->FindSet(fixture::kSetId);
    CHECK(set != nullptr);
    if (set)
        CHECK(snap.manifest.find("{\"id\":\"NPWR00001_00\",\"version\":\"" + set->version + "\"}") !=
              std::string::npos);
    CHECK_EQ(snap.sets.size(), size_t(1));
    std::string set_json = snap.sets[fixture::kSetId];
    CHECK(set_json.find("\"name\":\"Astro Teste\"") != std::string::npos);
    CHECK(set_json.find("\"earned\":true,\"earnedAt\":\"2025-11-07T20:00:00Z\"") != std::string::npos);
    CHECK(set_json.find("\"grade\":\"platinum\"") != std::string::npos);

    Snapshot same = BuildSnapshot(*lib, "tester", "pt-BR", hash);
    CHECK_EQ(same.fingerprint, snap.fingerprint);
    CHECK(hidden::Set("PPSA05555", true));
    Snapshot hidden_one = BuildSnapshot(*lib, "tester", "pt-BR", hash);
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
    for (const auto &[k, v] : r.headers)
        if (k == name) return v;
    return "";
}
}

TEST(cloud_client_requests_a_pairing_code) {
    FakeServer server;
    server.handler = [](const net::HttpsRequest &) {
        return Reply(
            200,
            R"({"deviceCode":"dc","userCode":"K7Q4-PX2A","verificationUri":"https://openpsapp.test/link","expiresIn":600,"interval":5})");
    };
    CloudClient client("https://api.openpsapp.test/", server.transport());
    std::string err;
    auto code = client.RequestCode("tes\"ter", &err);
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
    CHECK_EQ(server.seen[0].body, std::string(R"({"profileName":"tes\"ter"})"));
    CHECK_EQ(Header(server.seen[0], "Content-Type"), std::string("application/json"));

    server.handler = [](const net::HttpsRequest &) { return Reply(429, R"({"error":"rate_limited"})"); };
    CHECK(!client.RequestCode("tester", &err).has_value());
    CHECK(!err.empty());
}

TEST(cloud_client_maps_token_polling_answers) {
    FakeServer server;
    CloudClient client("https://api.openpsapp.test", server.transport());
    server.handler = [](const net::HttpsRequest &) { return Reply(428, R"({"error":"authorization_pending"})"); };
    CHECK(client.PollToken("dc", "tester").state == TokenState::Pending);
    server.handler = [](const net::HttpsRequest &) { return Reply(410); };
    CHECK(client.PollToken("dc", "tester").state == TokenState::Expired);
    server.handler = [](const net::HttpsRequest &) { return Reply(404); };
    CHECK(client.PollToken("dc", "tester").state == TokenState::Invalid);
    server.handler = [](const net::HttpsRequest &) {
        net::HttpsResponse r;
        r.error = "timeout: connect";
        return r;
    };
    TokenResult failed = client.PollToken("dc", "tester");
    CHECK(failed.state == TokenState::Failed);
    CHECK_EQ(failed.error, std::string("timeout: connect"));
    server.handler = [](const net::HttpsRequest &) { return Reply(200, R"({"deviceId":"dev-1","token":"opsa_abc"})"); };
    TokenResult granted = client.PollToken("dc", "tester");
    CHECK(granted.state == TokenState::Granted);
    CHECK_EQ(granted.link.token, std::string("opsa_abc"));
    CHECK_EQ(granted.link.device_id, std::string("dev-1"));
    CHECK_EQ(granted.link.profile_name, std::string("tester"));
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

TEST(cloud_client_skips_sets_and_images_the_server_refuses) {
    std::string image = TempPath("refused.png");
    CHECK(fs::WriteFileAtomic(image, "image-bytes"));
    std::string big(64, 'a'), odd(64, 'b'), stray(64, 'c'), fine(64, 'd');
    Snapshot snap;
    snap.manifest = "{}";
    snap.sets = {{"NPWR00001_00", "{}"}, {"NPWR00002_00", "{}"}};
    for (const std::string &id : {big, odd, stray, fine}) snap.assets[id] = {image, 0, -1, "1-1"};

    FakeServer server;
    server.handler = [&](const net::HttpsRequest &r) {
        if (r.url.find("/sync/manifest") != std::string::npos)
            return Reply(200, R"({"needSets":["NPWR00001_00","NPWR00002_00"],"needAssets":[")" + big + R"(",")" + odd +
                                  R"(",")" + stray + R"(",")" + fine + R"("]})");
        if (r.url.find("NPWR00001_00") != std::string::npos) return Reply(400, R"({"error":"invalid_set"})");
        if (r.url.find("/sync/sets/") != std::string::npos) return Reply(204);
        if (r.url.find(big) != std::string::npos) return Reply(413, R"({"error":"too_large"})");
        if (r.url.find(odd) != std::string::npos) return Reply(415, R"({"error":"unsupported_image"})");
        if (r.url.find(stray) != std::string::npos) return Reply(409, R"({"error":"not_referenced"})");
        return Reply(201);
    };
    CloudClient client("https://api.openpsapp.test", server.transport());
    auto read = [](const icons::Location &where, std::string *out) { return fs::ReadFile(where.path, out); };
    SyncResult result = client.Sync("opsa_tok", snap, read);
    CHECK(result.ok);
    CHECK_EQ(result.sets, 1);
    CHECK_EQ(result.refused_sets, 1);
    CHECK_EQ(result.assets, 1);
    CHECK_EQ(result.refused_assets, 3);
    CHECK_EQ(server.seen.size(), size_t(7));
}

TEST(cloud_client_stops_when_the_server_cannot_take_more) {
    std::string image = TempPath("stop.png");
    CHECK(fs::WriteFileAtomic(image, "image-bytes"));
    std::string first(64, 'a'), second(64, 'b');
    Snapshot snap;
    snap.manifest = "{}";
    snap.assets = {{first, {image, 0, -1, "1-1"}}, {second, {image, 0, -1, "1-1"}}};
    auto read = [](const icons::Location &where, std::string *out) { return fs::ReadFile(where.path, out); };
    for (int status : {429, 500, 507}) {
        FakeServer server;
        server.handler = [&](const net::HttpsRequest &r) {
            if (r.url.find("/sync/manifest") != std::string::npos)
                return Reply(200, R"({"needSets":[],"needAssets":[")" + first + R"(",")" + second + R"("]})");
            return Reply(status, R"({"error":"x"})");
        };
        CloudClient client("https://api.openpsapp.test", server.transport());
        SyncResult result = client.Sync("opsa_tok", snap, read);
        CHECK(!result.ok);
        CHECK_EQ(result.refused_assets, 0);
        CHECK_EQ(server.seen.size(), size_t(2));
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
        if (r.url.find("/sync/manifest") != std::string::npos)
            return Reply(200, R"({"needSets":["NPWR00001_00"],"needAssets":[]})");
        return Reply(500, R"({"error":"internal"})");
    };
    SyncResult failed = client.Sync("opsa_tok", snap, read);
    CHECK(!failed.ok);
    CHECK(!failed.unauthorized);
    CHECK(!failed.error.empty());
}

TEST(cloud_client_revokes_its_own_token) {
    FakeServer server;
    CloudClient client("https://api.openpsapp.test", server.transport());
    std::string err;
    for (int status : {204, 401, 404}) {
        server.handler = [status](const net::HttpsRequest &) { return Reply(status); };
        CHECK(client.Revoke("opsa_old", &err));
    }
    CHECK_EQ(server.seen.size(), size_t(3));
    CHECK_EQ(server.seen[0].method, std::string("DELETE"));
    CHECK_EQ(server.seen[0].url, std::string("https://api.openpsapp.test/api/v1/devices/self"));
    CHECK_EQ(Header(server.seen[0], "Authorization"), std::string("Bearer opsa_old"));
    CHECK_EQ(Header(server.seen[0], "Content-Type"), std::string(""));
    CHECK(server.seen[0].body.empty());

    server.handler = [](const net::HttpsRequest &) { return Reply(503); };
    CHECK(!client.Revoke("opsa_old", &err));
    CHECK_EQ(err, std::string("HTTP 503"));
    server.handler = [](const net::HttpsRequest &) {
        net::HttpsResponse r;
        r.error = "timeout: connect";
        return r;
    };
    CHECK(!client.Revoke("opsa_old", &err));
    CHECK_EQ(err, std::string("timeout: connect"));
}

namespace {
constexpr int64_t kMinute = 60 * 1000;

struct ControllerRig {
    FakeServer server;
    CloudClient client{"https://api.openpsapp.test", server.transport()};
    std::string path;
    CloudStore store;
    std::string fingerprint = "f1";
    std::string token = "opsa_tok";
    int manifests = 0;
    int token_polls = 0;
    int revokes = 0;
    int manifest_status = 200;
    int token_status = 428;
    int revoke_status = 204;
    bool offline = false;
    int expires_in = 600;
    CloudController controller;

    explicit ControllerRig(const char *store_file)
        : path(TempPath(store_file)), store(path),
          controller(
              &client, &store,
              SnapshotSource{[] { return std::optional<std::pair<std::string, std::string>>({"1eb71bbd", "tester"}); },
                             [this] {
                                 Snapshot snap;
                                 snap.manifest = "{}";
                                 snap.fingerprint = fingerprint;
                                 return snap;
                             },
                             [](const icons::Location &, std::string *) { return false; }}) {
        server.handler = [this](const net::HttpsRequest &r) {
            if (r.url.find("/devices/self") != std::string::npos) {
                revokes++;
                net::HttpsResponse down;
                down.error = "timeout: connect";
                return offline ? down : Reply(revoke_status);
            }
            if (r.url.find("/devices/codes") != std::string::npos)
                return Reply(
                    200,
                    R"({"deviceCode":"dc","userCode":"K7Q4-PX2A","verificationUri":"https://openpsapp.test/link","expiresIn":)" +
                        std::to_string(expires_in) + R"(,"interval":5})");
            if (r.url.find("/devices/token") != std::string::npos) {
                token_polls++;
                return token_status == 200 ? Reply(200, R"({"deviceId":"dev-1","token":")" + token + R"("})")
                                           : Reply(token_status);
            }
            if (r.url.find("/sync/manifest") != std::string::npos) {
                manifests++;
                return manifest_status == 200 ? Reply(200, R"({"needSets":[],"needAssets":[]})")
                                              : Reply(manifest_status);
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

SnapshotSource NoProfile() {
    return {[] { return std::optional<std::pair<std::string, std::string>>(); }, [] { return Snapshot{}; },
            [](const icons::Location &, std::string *) { return false; }};
}

bool IsRevoke(const net::HttpsRequest &r, const std::string &token) {
    return r.method == "DELETE" && r.url == "https://api.openpsapp.test/api/v1/devices/self" &&
           Header(r, "Authorization") == "Bearer " + token;
}

struct RouteRig : ControllerRig {
    http::Router router;

    explicit RouteRig(const char *store_file) : ControllerRig(store_file) {
        routes::RegisterCloud(router, 7777, [this] { return &controller; });
    }

    http::Response Send(const std::string &method, const std::string &path, const std::string &peer = "127.0.0.1",
                        const std::string &host = "127.0.0.1:7777", bool marked = true) {
        http::Request req;
        req.method = method;
        req.path = path;
        req.peer = peer;
        if (!host.empty()) req.headers["host"] = host;
        if (marked) req.headers["x-openpsapp"] = "1";
        return router.Dispatch(req);
    }
};

bool Has(const http::Response &res, const std::string &text) { return res.body.find(text) != std::string::npos; }
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
    CHECK_EQ(linked.profile_name, std::string("tester"));
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

TEST(cloud_controller_revokes_its_token_on_the_tick_after_unlinking) {
    ControllerRig rig("controller-unlink-revoke.json");
    rig.Link(0);
    size_t requests = rig.server.seen.size();
    rig.controller.Unlink();
    CHECK_EQ(rig.server.seen.size(), requests);
    CHECK_EQ(rig.revokes, 0);
    CHECK(!rig.controller.Status().linked);
    CHECK(rig.store.Revocations() == std::vector<std::string>{"opsa_tok"});
    CHECK(CloudStore(rig.path).Revocations() == std::vector<std::string>{"opsa_tok"});

    rig.controller.Tick(6000);
    CHECK_EQ(rig.revokes, 1);
    CHECK(IsRevoke(rig.server.seen.back(), "opsa_tok"));
    CHECK(rig.store.Revocations().empty());
    CHECK(CloudStore(rig.path).Revocations().empty());
    rig.controller.Tick(30 * kMinute);
    CHECK_EQ(rig.revokes, 1);
}

TEST(cloud_controller_never_revokes_for_a_console_that_was_not_linked) {
    ControllerRig rig("controller-unlink-none.json");
    rig.controller.Unlink();
    std::string err;
    CHECK(rig.controller.StartLink(0, &err));
    rig.controller.Unlink();
    rig.controller.Tick(kMinute);
    CHECK_EQ(rig.revokes, 0);
    for (const auto &r : rig.server.seen) CHECK(r.method != "DELETE");
    CHECK(rig.store.Revocations().empty());
}

TEST(cloud_controller_retries_a_failed_revoke_with_backoff) {
    ControllerRig rig("controller-revoke-retry.json");
    rig.Link(0);
    rig.controller.Tick(6000);
    rig.offline = true;
    rig.controller.Unlink();
    CHECK_EQ(rig.revokes, 0);
    rig.controller.Tick(7000);
    CHECK_EQ(rig.revokes, 1);
    CHECK(!rig.controller.Status().linked);
    CHECK(rig.store.Revocations() == std::vector<std::string>{"opsa_tok"});
    CHECK(CloudStore(rig.path).Revocations() == std::vector<std::string>{"opsa_tok"});

    rig.controller.Tick(9000);
    CHECK_EQ(rig.revokes, 1);
    rig.offline = false;
    rig.revoke_status = 503;
    rig.controller.Tick(7000 + kMinute);
    CHECK_EQ(rig.revokes, 2);
    rig.controller.Tick(7000 + 2 * kMinute);
    CHECK_EQ(rig.revokes, 2);
    rig.revoke_status = 204;
    rig.controller.Tick(7000 + 3 * kMinute);
    CHECK_EQ(rig.revokes, 3);
    CHECK(IsRevoke(rig.server.seen.back(), "opsa_tok"));
    CHECK(rig.store.Revocations().empty());
    rig.controller.Tick(7000 + 30 * kMinute);
    CHECK_EQ(rig.revokes, 3);
}

TEST(cloud_controller_retries_a_pending_revoke_after_a_restart) {
    ControllerRig rig("controller-revoke-restart.json");
    rig.Link(0);
    rig.offline = true;
    rig.controller.Unlink();
    rig.controller.Tick(6000);
    CHECK_EQ(rig.revokes, 1);

    rig.offline = false;
    rig.revoke_status = 401;
    CloudStore reloaded(rig.path);
    CHECK(reloaded.Revocations() == std::vector<std::string>{"opsa_tok"});
    CHECK(!reloaded.Get("1eb71bbd").has_value());
    CloudController restarted(&rig.client, &reloaded, NoProfile());
    restarted.Tick(0);
    CHECK_EQ(rig.revokes, 2);
    CHECK(IsRevoke(rig.server.seen.back(), "opsa_tok"));
    CHECK(reloaded.Revocations().empty());
    CHECK(CloudStore(rig.path).Revocations().empty());
}

TEST(cloud_controller_keeps_a_new_link_apart_from_an_old_pending_revoke) {
    ControllerRig rig("controller-revoke-relink.json");
    rig.Link(0);
    rig.offline = true;
    rig.controller.Unlink();
    rig.token = "opsa_new";
    rig.Link(10000);
    CHECK(rig.controller.Status().linked);
    rig.controller.Tick(16000);
    CHECK_EQ(rig.manifests, 1);
    CHECK_EQ(Header(rig.server.seen.back(), "Authorization"), std::string("Bearer opsa_new"));

    rig.offline = false;
    rig.controller.Tick(15000 + kMinute);
    CHECK_EQ(rig.revokes, 2);
    CHECK(IsRevoke(rig.server.seen.back(), "opsa_tok"));
    for (const auto &r : rig.server.seen) CHECK(!IsRevoke(r, "opsa_new"));
    auto stored = rig.store.Get("1eb71bbd");
    CHECK(stored.has_value() && stored->token == "opsa_new");
    CHECK(rig.store.Revocations().empty());
    CHECK(rig.controller.Status().linked);
}

TEST(cloud_controller_ignores_a_sync_that_outlived_its_link) {
    ControllerRig rig("controller-stale-sync.json");
    rig.Link(0);
    auto normal = rig.server.handler;
    rig.server.handler = [&](const net::HttpsRequest &r) {
        if (r.url.find("/sync/manifest") == std::string::npos) return normal(r);
        rig.controller.Unlink();
        rig.store.Put("1eb71bbd", {"opsa_new", "dev-2", "tester"});
        return Reply(401);
    };
    rig.controller.Tick(6000);
    CHECK_EQ(rig.revokes, 0);
    CHECK(rig.store.Revocations() == std::vector<std::string>{"opsa_tok"});
    auto stored = rig.store.Get("1eb71bbd");
    CHECK(stored.has_value() && stored->token == "opsa_new");
    CHECK(rig.controller.Status().linked);
    CHECK(rig.controller.Status().last_error.empty());
    rig.server.handler = normal;
    rig.controller.Tick(8000);
    CHECK_EQ(rig.revokes, 1);
    bool revoked_old = false;
    for (const auto &r : rig.server.seen) revoked_old = revoked_old || IsRevoke(r, "opsa_tok");
    CHECK(revoked_old);
    for (const auto &r : rig.server.seen) CHECK(!IsRevoke(r, "opsa_new"));
}

TEST(cloud_controller_retries_images_it_could_not_read) {
    ControllerRig rig("controller-unread.json");
    std::string asset_id(64, 'c');
    bool readable = false;
    int reads = 0;
    int uploads = 0;
    SnapshotSource source;
    source.user = [] { return std::optional<std::pair<std::string, std::string>>({"1eb71bbd", "tester"}); };
    source.build = [&] {
        Snapshot snap;
        snap.manifest = "{}";
        snap.fingerprint = "f1";
        snap.assets[asset_id] = {"/img.png", 0, -1, "1-1"};
        return snap;
    };
    source.read = [&](const icons::Location &, std::string *out) {
        reads++;
        *out = "png";
        return readable;
    };
    CloudController controller(&rig.client, &rig.store, source);
    auto normal = rig.server.handler;
    rig.server.handler = [&](const net::HttpsRequest &r) {
        if (r.url.find("/sync/manifest") != std::string::npos) {
            rig.manifests++;
            return Reply(200, R"({"needSets":[],"needAssets":[")" + asset_id + R"("]})");
        }
        if (r.url.find("/sync/assets/") != std::string::npos) {
            uploads++;
            return Reply(201);
        }
        return normal(r);
    };
    std::string err;
    CHECK(controller.StartLink(0, &err));
    rig.token_status = 200;
    controller.Tick(5000);
    controller.Tick(6000);
    CHECK_EQ(rig.manifests, 1);
    CHECK_EQ(reads, 1);
    CHECK_EQ(uploads, 0);
    CHECK(!controller.Status().last_sync_at.empty());

    controller.Tick(6000 + kMinute + 1000);
    CHECK_EQ(rig.manifests, 2);
    CHECK_EQ(reads, 2);
    CHECK_EQ(uploads, 0);

    readable = true;
    controller.Tick(6000 + 2 * kMinute + 2000);
    CHECK_EQ(rig.manifests, 3);
    CHECK_EQ(uploads, 1);
    controller.Tick(6000 + 3 * kMinute + 3000);
    CHECK_EQ(rig.manifests, 3);
}

TEST(cloud_controller_refuses_to_relink_a_linked_profile) {
    RouteRig rig("controller-relink.json");
    rig.Link(0);
    size_t requests = rig.server.seen.size();
    std::string err;
    CHECK(!rig.controller.StartLink(10000, &err));
    CHECK_EQ(err, std::string("already_linked"));
    CHECK_EQ(rig.server.seen.size(), requests);
    CHECK(!rig.controller.Status().pairing.has_value());

    http::Response res = rig.Send("POST", "/api/v1/cloud/link");
    CHECK_EQ(res.status, 409);
    CHECK_EQ(res.body, std::string(R"({"error":"already_linked"})"));
    CHECK_EQ(rig.server.seen.size(), requests);
    auto stored = rig.store.Get("1eb71bbd");
    CHECK(stored.has_value() && stored->token == "opsa_tok");
}

TEST(cloud_routes_report_link_failures_as_codes) {
    RouteRig rig("routes-link-fail.json");
    rig.server.handler = [](const net::HttpsRequest &) {
        net::HttpsResponse r;
        r.error = "timeout: connect";
        return r;
    };
    http::Response down = rig.Send("POST", "/api/v1/cloud/link");
    CHECK_EQ(down.status, 502);
    CHECK_EQ(down.body, std::string(R"({"error":"cloud_unavailable"})"));
    CHECK_EQ(rig.controller.Status().last_error, std::string("timeout: connect"));

    CloudController signed_out(&rig.client, &rig.store, NoProfile());
    http::Router router;
    routes::RegisterCloud(router, 7777, [&signed_out] { return &signed_out; });
    http::Request req;
    req.method = "POST";
    req.path = "/api/v1/cloud/link";
    req.peer = "127.0.0.1";
    req.headers = {{"host", "127.0.0.1:7777"}, {"x-openpsapp", "1"}};
    http::Response nobody = router.Dispatch(req);
    CHECK_EQ(nobody.status, 409);
    CHECK_EQ(nobody.body, std::string(R"({"error":"no_profile"})"));
}

TEST(cloud_routes_link_from_the_console_screen) {
    RouteRig rig("routes-local.json");
    http::Response linked = rig.Send("POST", "/api/v1/cloud/link");
    CHECK_EQ(linked.status, 200);
    CHECK(Has(linked, "\"local\":true"));
    CHECK(Has(linked, "\"userCode\":\"K7Q4-PX2A\""));

    http::Response status = rig.Send("GET", "/api/v1/cloud", "127.0.0.1", "localhost:7777", false);
    CHECK_EQ(status.status, 200);
    CHECK(Has(status, "\"local\":true"));
    CHECK(Has(status, "\"verificationUri\":\"https://openpsapp.test/link\""));

    CHECK_EQ(rig.Send("POST", "/api/v1/cloud/unlink", "127.0.0.2", "localhost:7777").status, 200);
    CHECK(!rig.controller.Status().pairing.has_value());
    rig.Link(20000);
    CHECK_EQ(rig.Send("POST", "/api/v1/cloud/sync").status, 200);
}

TEST(cloud_routes_refuse_other_devices_on_the_lan) {
    RouteRig rig("routes-lan.json");
    for (const char *action : {"link", "unlink", "sync"}) {
        http::Response res = rig.Send("POST", std::string("/api/v1/cloud/") + action, "192.168.1.20");
        CHECK_EQ(res.status, 403);
        CHECK_EQ(res.body, std::string(R"({"error":"console_only"})"));
    }
    CHECK_EQ(rig.Send("POST", "/api/v1/cloud/link", "").status, 403);
    CHECK(rig.server.seen.empty());

    rig.Link(0);
    CHECK_EQ(rig.Send("POST", "/api/v1/cloud/unlink", "192.168.1.20", "192.168.1.15:7777").status, 403);
    CHECK(rig.controller.Status().linked);
}

TEST(cloud_routes_refuse_a_foreign_host_header) {
    RouteRig rig("routes-host.json");
    for (const char *host :
         {"evil.example:7777", "127.0.0.1:8080", "127.0.0.1", "localhost", "192.168.1.15:7777", ""}) {
        http::Response res = rig.Send("POST", "/api/v1/cloud/link", "127.0.0.1", host);
        CHECK_EQ(res.status, 403);
    }
    CHECK(rig.server.seen.empty());
    CHECK(Has(rig.Send("GET", "/api/v1/cloud", "127.0.0.1", "evil.example:7777", false), "\"local\":false"));
}

TEST(cloud_routes_require_the_openpsapp_header) {
    RouteRig rig("routes-marker.json");
    CHECK_EQ(rig.Send("POST", "/api/v1/cloud/link", "127.0.0.1", "127.0.0.1:7777", false).status, 403);
    http::Request wrong;
    wrong.method = "POST";
    wrong.path = "/api/v1/cloud/link";
    wrong.peer = "127.0.0.1";
    wrong.headers = {{"host", "127.0.0.1:7777"}, {"x-openpsapp", "0"}};
    CHECK_EQ(rig.router.Dispatch(wrong).status, 403);
    CHECK_EQ(rig.Send("OPTIONS", "/api/v1/cloud/link").status, 405);
    CHECK(rig.server.seen.empty());
}

TEST(cloud_status_hides_the_pairing_code_from_other_devices) {
    RouteRig rig("routes-status.json");
    CHECK_EQ(rig.Send("POST", "/api/v1/cloud/link").status, 200);

    http::Response remote = rig.Send("GET", "/api/v1/cloud", "192.168.1.20", "192.168.1.15:7777", false);
    CHECK_EQ(remote.status, 200);
    CHECK(Has(remote, "\"local\":false"));
    CHECK(Has(remote, "\"pairing\":{\"expiresIn\":"));
    CHECK(!Has(remote, "userCode"));
    CHECK(!Has(remote, "verificationUri"));
    CHECK(!Has(remote, "K7Q4"));
    CHECK(!Has(remote, "openpsapp.test"));

    http::Response local = rig.Send("GET", "/api/v1/cloud", "127.0.0.1", "127.0.0.1:7777", false);
    CHECK(Has(local, "\"local\":true"));
    CHECK(Has(local, "\"userCode\":\"K7Q4-PX2A\""));
}

TEST(cloud_routes_report_an_unconfigured_cloud) {
    test::App app;
    http::Response status = test::Get(app.router, "/api/v1/cloud");
    CHECK_EQ(status.status, 200);
    CHECK(status.body.find("\"configured\":false") != std::string::npos);
    CHECK(status.body.find("\"local\":false") != std::string::npos);
    CHECK_EQ(test::Call(app.router, "POST", "/api/v1/cloud/link").status, 403);
    http::Request req;
    req.method = "POST";
    req.path = "/api/v1/cloud/link";
    req.peer = "127.0.0.1";
    req.headers = {{"host", "127.0.0.1:7777"}, {"x-openpsapp", "1"}};
    CHECK_EQ(app.router.Dispatch(req).status, 409);
}
