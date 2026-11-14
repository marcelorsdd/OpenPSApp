#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace openpsapp::cloud {
struct CloudLink {
    std::string token;
    std::string device_id;
    std::string profile_name;
};

class CloudStore {
public:
    explicit CloudStore(std::string path);

    std::optional<CloudLink> Get(const std::string &user) const;
    bool Put(const std::string &user, const CloudLink &link);
    bool Remove(const std::string &user);
    std::optional<std::string> Retire(const std::string &user);
    std::vector<std::string> Revocations() const { return revoke_; }
    bool Revoked(const std::string &token);

private:
    bool Save() const;

    std::string path_;
    std::map<std::string, CloudLink> links_;
    std::vector<std::string> revoke_;
};

}
