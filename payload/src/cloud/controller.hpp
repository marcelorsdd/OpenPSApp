#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

#include "cloud/client.hpp"
#include "cloud/snapshot.hpp"
#include "cloud/store.hpp"

namespace openpsapp::cloud {
struct PairingView {
    std::string user_code;
    std::string verification_uri;
    int64_t expires_at_ms = 0;
};

struct CloudStatus {
    bool configured = true;
    bool linked = false;
    bool syncing = false;
    std::string profile_name;
    std::string last_sync_at;
    std::string last_error;
    std::optional<PairingView> pairing;
};

struct SnapshotSource {
    std::function<std::optional<std::pair<std::string, std::string>>()> user;
    std::function<Snapshot()> build;
    ReadAssetFn read;
};

class CloudController {
public:
    CloudController(CloudClient *client, CloudStore *store, SnapshotSource source);

    CloudStatus Status() const;
    bool StartLink(int64_t now_ms, std::string *err);
    void Unlink();
    void RequestSync();
    void Tick(int64_t now_ms);

private:
    struct Pairing {
        std::string user_key;
        std::string profile_name;
        std::string device_code;
        std::string user_code;
        std::string verification_uri;
        int64_t expires_at_ms = 0;
        int64_t next_poll_ms = 0;
        int64_t interval_ms = 5000;
    };

    bool TickPairing(int64_t now_ms);
    void TickSync(int64_t now_ms, const std::string &user_key);
    void TickRevokes(int64_t now_ms);
    void FinishRevoke(const std::string &token, bool done, const std::string &err, int64_t now_ms);
    void ResetSchedule();

    CloudClient *client_;
    CloudStore *store_;
    SnapshotSource source_;

    mutable std::mutex mu_;
    std::optional<Pairing> pairing_;
    std::string user_key_;
    bool sync_requested_ = false;
    bool syncing_ = false;
    int64_t last_sync_ms_ = -1;
    int64_t last_check_ms_ = -1;
    int64_t retry_at_ms_ = 0;
    std::string last_fingerprint_;
    std::string last_sync_at_;
    std::string last_error_;
    int revoke_failures_ = 0;
    int64_t revoke_at_ms_ = 0;
};

}
