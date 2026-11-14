#include "cloud/controller.hpp"

#include <algorithm>

#include "util/log.hpp"
#include "util/timeutil.hpp"

namespace openpsapp::cloud {
namespace {
constexpr int64_t kSyncEveryMs = 15 * 60 * 1000;
constexpr int64_t kCheckEveryMs = 60 * 1000;
constexpr int64_t kMaxRevokeBackoffMs = 60 * 60 * 1000;

int64_t RevokeBackoff(int failures) {
    return std::min(kCheckEveryMs << std::min(failures - 1, 6), kMaxRevokeBackoffMs);
}
}

CloudController::CloudController(CloudClient *client, CloudStore *store, SnapshotSource source)
    : client_(client), store_(store), source_(std::move(source)) {}

CloudStatus CloudController::Status() const {
    auto user = source_.user();
    std::lock_guard<std::mutex> lock(mu_);
    CloudStatus status;
    status.syncing = syncing_;
    status.last_sync_at = last_sync_at_;
    status.last_error = last_error_;
    if (pairing_)
        status.pairing = PairingView{pairing_->user_code, pairing_->verification_uri, pairing_->expires_at_ms};
    if (!user) return status;
    if (auto link = store_->Get(user->first)) {
        status.linked = true;
        status.profile_name = link->profile_name;
    } else {
        status.profile_name = user->second;
    }
    return status;
}

bool CloudController::StartLink(int64_t now_ms, std::string *err) {
    auto user = source_.user();
    if (!user) {
        *err = "no_profile";
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (store_->Get(user->first)) {
            *err = "already_linked";
            return false;
        }
    }
    std::string failure;
    auto code = client_->RequestCode(user->second, &failure);
    std::lock_guard<std::mutex> lock(mu_);
    if (!code) {
        last_error_ = failure;
        *err = "cloud_unavailable";
        return false;
    }
    int64_t interval_ms = int64_t(code->interval) * 1000;
    pairing_ = Pairing{user->first,
                       user->second,
                       code->device_code,
                       code->user_code,
                       code->verification_uri,
                       now_ms + int64_t(code->expires_in) * 1000,
                       now_ms + interval_ms,
                       interval_ms};
    last_error_.clear();
    return true;
}

void CloudController::Unlink() {
    auto user = source_.user();
    std::lock_guard<std::mutex> lock(mu_);
    pairing_.reset();
    if (user && store_->Retire(user->first)) revoke_at_ms_ = 0;
    ResetSchedule();
    last_sync_at_.clear();
    last_error_.clear();
}

void CloudController::RequestSync() {
    std::lock_guard<std::mutex> lock(mu_);
    sync_requested_ = true;
    retry_at_ms_ = 0;
}

void CloudController::Tick(int64_t now_ms) {
    TickRevokes(now_ms);
    auto user = source_.user();
    if (!user) return;
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (user->first != user_key_) {
            user_key_ = user->first;
            ResetSchedule();
        }
    }
    if (TickPairing(now_ms)) return;
    TickSync(now_ms, user->first);
}

bool CloudController::TickPairing(int64_t now_ms) {
    Pairing pairing;
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (!pairing_) return false;
        if (now_ms >= pairing_->expires_at_ms) {
            pairing_.reset();
            last_error_ = "pairing code expired";
            return true;
        }
        if (now_ms < pairing_->next_poll_ms) return true;
        pairing = *pairing_;
    }
    TokenResult result = client_->PollToken(pairing.device_code, pairing.profile_name);
    std::lock_guard<std::mutex> lock(mu_);
    if (!pairing_ || pairing_->device_code != pairing.device_code) return true;
    switch (result.state) {
    case TokenState::Pending: pairing_->next_poll_ms = now_ms + pairing.interval_ms; break;
    case TokenState::Failed:
        pairing_->next_poll_ms = now_ms + pairing.interval_ms;
        last_error_ = result.error;
        break;
    case TokenState::Expired:
    case TokenState::Invalid:
        pairing_.reset();
        last_error_ = "pairing code expired";
        break;
    case TokenState::Granted:
        store_->Put(pairing.user_key, result.link);
        pairing_.reset();
        last_error_.clear();
        ResetSchedule();
        sync_requested_ = true;
        break;
    }
    return true;
}

void CloudController::TickSync(int64_t now_ms, const std::string &user_key) {
    std::optional<CloudLink> link;
    bool requested = false;
    bool due_periodic = false;
    {
        std::lock_guard<std::mutex> lock(mu_);
        link = store_->Get(user_key);
        if (!link || now_ms < retry_at_ms_) return;
        requested = sync_requested_;
        due_periodic = last_sync_ms_ < 0 || now_ms - last_sync_ms_ >= kSyncEveryMs;
        bool due_check = last_check_ms_ < 0 || now_ms - last_check_ms_ >= kCheckEveryMs;
        if (!requested && !due_periodic && !due_check) return;
    }
    Snapshot snapshot = source_.build();
    {
        std::lock_guard<std::mutex> lock(mu_);
        last_check_ms_ = now_ms;
        if (!requested && !due_periodic && snapshot.fingerprint == last_fingerprint_) return;
        syncing_ = true;
        sync_requested_ = false;
    }
    SyncResult result = client_->Sync(link->token, snapshot, source_.read);
    std::lock_guard<std::mutex> lock(mu_);
    syncing_ = false;
    auto current = store_->Get(user_key);
    if (!current || current->token != link->token) return;
    if (result.ok) {
        last_sync_ms_ = now_ms;
        last_fingerprint_ = result.unread_assets == 0 ? snapshot.fingerprint : "";
        last_sync_at_ = timeutil::FormatIsoOrEmpty(timeutil::NowUnix());
        last_error_.clear();
        retry_at_ms_ = 0;
        if (result.unread_assets > 0)
            log::Error("cloud: %d image(s) could not be read; retrying", result.unread_assets);
    } else if (result.unauthorized) {
        store_->Remove(user_key);
        ResetSchedule();
        last_error_ = "console unlinked on the website";
    } else {
        last_error_ = result.error;
        retry_at_ms_ = now_ms + kCheckEveryMs;
    }
}

void CloudController::TickRevokes(int64_t now_ms) {
    std::vector<std::string> tokens;
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (now_ms < revoke_at_ms_) return;
        tokens = store_->Revocations();
    }
    for (const std::string &token : tokens) {
        std::string err;
        bool done = client_->Revoke(token, &err);
        std::lock_guard<std::mutex> lock(mu_);
        FinishRevoke(token, done, err, now_ms);
        if (!done) break;
    }
}

void CloudController::FinishRevoke(const std::string &token, bool done, const std::string &err, int64_t now_ms) {
    if (done) {
        store_->Revoked(token);
        revoke_failures_ = 0;
        revoke_at_ms_ = 0;
        return;
    }
    revoke_failures_++;
    revoke_at_ms_ = now_ms + RevokeBackoff(revoke_failures_);
    log::Error("cloud: could not revoke an old token (%s); retrying later", err.c_str());
}

void CloudController::ResetSchedule() {
    sync_requested_ = false;
    last_sync_ms_ = -1;
    last_check_ms_ = -1;
    retry_at_ms_ = 0;
    last_fingerprint_.clear();
}

}
