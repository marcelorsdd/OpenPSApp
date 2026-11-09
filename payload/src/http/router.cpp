#include "http/http.hpp"

#include "util/json.hpp"

namespace playtrack::http {
namespace {
std::vector<std::string> Split(std::string_view path) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= path.size()) {
        size_t end = path.find('/', start);
        if (end == std::string_view::npos) end = path.size();
        if (end > start) parts.emplace_back(path.substr(start, end - start));
        start = end + 1;
    }
    return parts;
}

int HexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
std::string UrlDecode(std::string_view in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size() && HexValue(in[i + 1]) >= 0 && HexValue(in[i + 2]) >= 0) {
            out += static_cast<char>(HexValue(in[i + 1]) * 16 + HexValue(in[i + 2]));
            i += 2;
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return out;
}
}

Response Json(std::string body, int status) {
    Response res;
    res.status = status;
    res.body = std::move(body);
    return res;
}

Response Error(int status, std::string_view message) {
    JsonWriter w;
    w.BeginObject().Key("error").String(message).EndObject();
    return Json(w.take(), status);
}

Response File(std::string path, std::string content_type, uint64_t offset, int64_t length) {
    Response res;
    res.file_path = std::move(path);
    res.content_type = std::move(content_type);
    res.file_offset = offset;
    res.file_length = length;
    return res;
}

void ParseTarget(std::string_view target, std::string *path, std::map<std::string, std::string> *query) {
    size_t q = target.find('?');
    *path = UrlDecode(target.substr(0, q));
    if (q == std::string_view::npos) return;
    std::string_view rest = target.substr(q + 1);
    while (!rest.empty()) {
        size_t amp = rest.find('&');
        std::string_view pair = rest.substr(0, amp);
        size_t eq = pair.find('=');
        std::string key = UrlDecode(pair.substr(0, eq));
        std::string value = eq == std::string_view::npos ? "" : UrlDecode(pair.substr(eq + 1));
        if (!key.empty()) (*query)[key] = value;
        if (amp == std::string_view::npos) break;
        rest = rest.substr(amp + 1);
    }
}

void Router::Get(std::string pattern, Handler handler) {
    routes_.push_back({"GET", Split(pattern), std::move(handler)});
}

void Router::Post(std::string pattern, Handler handler) {
    routes_.push_back({"POST", Split(pattern), std::move(handler)});
}

Response Router::Dispatch(Request req) const {
    std::vector<std::string> parts = Split(req.path);
    std::string method = req.method == "HEAD" ? "GET" : req.method;
    bool path_matched = false;
    for (const Route &route : routes_) {
        if (route.segments.size() != parts.size()) continue;
        std::map<std::string, std::string> params;
        bool match = true;
        for (size_t i = 0; i < parts.size() && match; ++i) {
            const std::string &seg = route.segments[i];
            if (!seg.empty() && seg[0] == ':') params[seg.substr(1)] = parts[i];
            else match = seg == parts[i];
        }
        if (!match) continue;
        path_matched = true;
        if (route.method != method) continue;
        req.params = std::move(params);
        return route.handler(req);
    }
    return path_matched ? Error(405, "method not allowed") : Error(404, "not found");
}

}
