#include "cloud/client.hpp"

#include "cJSON.h"
#include "util/json.hpp"

namespace openpsapp::cloud {
namespace {
constexpr const char *kJson = "application/json";
constexpr int kAssetTimeoutMs = 120000;

std::string Failure(const net::HttpsResponse &r) {
    if (!r.error.empty()) return r.error;
    return "HTTP " + std::to_string(r.status);
}

bool Refused(const net::HttpsResponse &r) {
    return r.ok() && (r.status == 400 || r.status == 409 || r.status == 413 || r.status == 415);
}

std::vector<std::string> StringArray(const cJSON *obj, const char *name) {
    std::vector<std::string> out;
    const cJSON *item = nullptr;
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(obj, name)) {
        if (cJSON_IsString(item)) out.push_back(item->valuestring);
    }
    return out;
}
}

CloudClient::CloudClient(std::string server, Transport transport)
    : server_(std::move(server)), transport_(std::move(transport)) {
    while (!server_.empty() && server_.back() == '/') server_.pop_back();
}

net::HttpsResponse CloudClient::Call(const char *method, const std::string &path, std::string body,
                                     const std::string &token, const char *content_type, int timeout_ms) {
    net::HttpsRequest req;
    req.method = method;
    req.url = server_ + path;
    req.body = std::move(body);
    req.timeout_ms = timeout_ms;
    req.headers.push_back({"User-Agent", "OpenPSApp"});
    if (content_type) req.headers.push_back({"Content-Type", content_type});
    if (!token.empty()) req.headers.push_back({"Authorization", "Bearer " + token});
    return transport_(req);
}

std::optional<PairingCode> CloudClient::RequestCode(const std::string &profile_name, std::string *err) {
    JsonWriter w;
    w.BeginObject().String("profileName", profile_name).EndObject();
    net::HttpsResponse r = Call("POST", "/api/v1/devices/codes", w.take(), "", kJson);
    if (!r.ok() || r.status != 200) {
        *err = Failure(r);
        return std::nullopt;
    }
    cJSON *root = cJSON_ParseWithLength(r.body.data(), r.body.size());
    PairingCode code;
    code.device_code = JsonString(root, "deviceCode");
    code.user_code = JsonString(root, "userCode");
    code.verification_uri = JsonString(root, "verificationUri");
    if (const cJSON *v = cJSON_GetObjectItemCaseSensitive(root, "expiresIn"); cJSON_IsNumber(v))
        code.expires_in = v->valueint;
    if (const cJSON *v = cJSON_GetObjectItemCaseSensitive(root, "interval"); cJSON_IsNumber(v) && v->valueint > 0) {
        code.interval = v->valueint;
    }
    cJSON_Delete(root);
    if (code.device_code.empty() || code.user_code.empty()) {
        *err = "unexpected pairing answer";
        return std::nullopt;
    }
    return code;
}

TokenResult CloudClient::PollToken(const std::string &device_code, const std::string &profile_name) {
    JsonWriter w;
    w.BeginObject().String("deviceCode", device_code).EndObject();
    net::HttpsResponse r = Call("POST", "/api/v1/devices/token", w.take(), "", kJson);
    TokenResult out;
    if (!r.ok()) {
        out.error = r.error;
        return out;
    }
    if (r.status == 428) {
        out.state = TokenState::Pending;
    } else if (r.status == 410) {
        out.state = TokenState::Expired;
    } else if (r.status == 404) {
        out.state = TokenState::Invalid;
    } else if (r.status == 200) {
        cJSON *root = cJSON_ParseWithLength(r.body.data(), r.body.size());
        out.link = {JsonString(root, "token"), JsonString(root, "deviceId"), profile_name};
        cJSON_Delete(root);
        out.state = out.link.token.empty() ? TokenState::Failed : TokenState::Granted;
        if (out.link.token.empty()) out.error = "unexpected token answer";
    } else {
        out.error = Failure(r);
    }
    return out;
}

SyncResult CloudClient::Sync(const std::string &token, const Snapshot &snapshot, const ReadAssetFn &read) {
    SyncResult out;
    auto fail = [&out](const net::HttpsResponse &r) {
        out.unauthorized = r.ok() && r.status == 401;
        out.error = out.unauthorized ? "unauthorized" : Failure(r);
        return out;
    };

    net::HttpsResponse manifest = Call("POST", "/api/v1/sync/manifest", snapshot.manifest, token, kJson);
    if (!manifest.ok() || manifest.status != 200) return fail(manifest);
    cJSON *root = cJSON_ParseWithLength(manifest.body.data(), manifest.body.size());
    std::vector<std::string> need_sets = StringArray(root, "needSets");
    std::vector<std::string> need_assets = StringArray(root, "needAssets");
    cJSON_Delete(root);

    for (const std::string &id : need_sets) {
        auto set = snapshot.sets.find(id);
        if (set == snapshot.sets.end()) continue;
        net::HttpsResponse r = Call("PUT", "/api/v1/sync/sets/" + id, set->second, token, kJson);
        if (Refused(r)) {
            out.refused_sets++;
            continue;
        }
        if (!r.ok() || r.status / 100 != 2) return fail(r);
        out.sets++;
    }
    for (const std::string &id : need_assets) {
        auto asset = snapshot.assets.find(id);
        if (asset == snapshot.assets.end()) continue;
        std::string bytes;
        if (!read(asset->second, &bytes)) {
            out.unread_assets++;
            continue;
        }
        net::HttpsResponse r = Call("PUT", "/api/v1/sync/assets/" + id, std::move(bytes), token,
                                    "application/octet-stream", kAssetTimeoutMs);
        if (Refused(r)) {
            out.refused_assets++;
            continue;
        }
        if (!r.ok() || r.status / 100 != 2) return fail(r);
        out.assets++;
    }
    out.ok = true;
    return out;
}

bool CloudClient::Revoke(const std::string &token, std::string *err) {
    net::HttpsResponse r = Call("DELETE", "/api/v1/devices/self", "", token, nullptr);
    if (r.ok() && (r.status == 204 || r.status == 401 || r.status == 404)) return true;
    *err = Failure(r);
    return false;
}

}
