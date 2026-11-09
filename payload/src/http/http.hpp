#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace playtrack::http {
struct Request {
    std::string method;
    std::string path;
    std::map<std::string, std::string> query;
    std::map<std::string, std::string> params;
    std::string body;
};

struct Response {
    int status = 200;
    std::string content_type = "application/json";
    std::string body;
    std::string file_path;
    uint64_t file_offset = 0;
    int64_t file_length = -1;
    std::string cache_control = "no-store";
    std::string content_encoding;
};

constexpr const char *kCacheForever = "public, max-age=31536000, immutable";

Response Json(std::string body, int status = 200);
Response Error(int status, std::string_view message);
Response File(std::string path, std::string content_type, uint64_t offset = 0, int64_t length = -1);

void ParseTarget(std::string_view target, std::string *path, std::map<std::string, std::string> *query);

class Router {
public:
    using Handler = std::function<Response(const Request &)>;

    void Get(std::string pattern, Handler handler);
    void Post(std::string pattern, Handler handler);
    Response Dispatch(Request req) const;

private:
    struct Route {
        std::string method;
        std::vector<std::string> segments;
        Handler handler;
    };
    std::vector<Route> routes_;
};

}
