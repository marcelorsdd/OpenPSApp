#pragma once

#include <map>
#include <optional>
#include <string>

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

private:
    bool Save() const;

    std::string path_;
    std::map<std::string, CloudLink> links_;
};

}
