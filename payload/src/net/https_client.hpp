#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "net/http_message.hpp"

namespace openpsapp::net {
struct HttpsRequest {
    std::string method = "GET";
    std::string url;
    Headers headers;
    std::string body;
    int timeout_ms = 15000;
};

struct HttpsResponse {
    int status = 0;
    std::string body;
    std::string error;

    bool ok() const { return error.empty(); }
};

HttpsResponse HttpsSend(const HttpsRequest &request, const std::vector<std::string_view> &ca_pems);

}
