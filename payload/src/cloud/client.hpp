#pragma once

#include <functional>
#include <optional>
#include <string>

#include "cloud/snapshot.hpp"
#include "cloud/store.hpp"
#include "net/https_client.hpp"

namespace openpsapp::cloud {
using Transport = std::function<net::HttpsResponse(const net::HttpsRequest &)>;
using ReadAssetFn = std::function<bool(const icons::Location &, std::string *)>;

struct PairingCode {
    std::string device_code;
    std::string user_code;
    std::string verification_uri;
    int expires_in = 600;
    int interval = 5;
};

enum class TokenState { Pending, Granted, Expired, Invalid, Failed };

struct TokenResult {
    TokenState state = TokenState::Failed;
    CloudLink link;
    std::string error;
};

struct SyncResult {
    bool ok = false;
    bool unauthorized = false;
    int sets = 0;
    int assets = 0;
    std::string error;
};

class CloudClient {
public:
    CloudClient(std::string server, Transport transport);

    std::optional<PairingCode> RequestCode(const std::string &profile_name, std::string *err);
    TokenResult PollToken(const std::string &device_code, const std::string &profile_name);
    SyncResult Sync(const std::string &token, const Snapshot &snapshot, const ReadAssetFn &read);

private:
    net::HttpsResponse Call(const char *method, const std::string &path, std::string body, const std::string &token,
                            const char *content_type, int timeout_ms = 20000);

    std::string server_;
    Transport transport_;
};

}
