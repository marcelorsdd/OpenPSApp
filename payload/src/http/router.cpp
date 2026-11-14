#include "http/http.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cctype>

#include "util/json.hpp"

namespace openpsapp::http {
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
std::string UrlDecode(std::string_view in, bool plus_is_space) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size() && HexValue(in[i + 1]) >= 0 && HexValue(in[i + 2]) >= 0) {
            out += static_cast<char>(HexValue(in[i + 1]) * 16 + HexValue(in[i + 2]));
            i += 2;
        } else if (in[i] == '+' && plus_is_space) {
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

Response Error(int status, std::string_view code) {
    JsonWriter w;
    w.BeginObject().String("error", code).EndObject();
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
    *path = UrlDecode(target.substr(0, q), false);
    if (q == std::string_view::npos) return;
    std::string_view rest = target.substr(q + 1);
    while (!rest.empty()) {
        size_t amp = rest.find('&');
        std::string_view pair = rest.substr(0, amp);
        size_t eq = pair.find('=');
        std::string key = UrlDecode(pair.substr(0, eq), true);
        std::string value = eq == std::string_view::npos ? "" : UrlDecode(pair.substr(eq + 1), true);
        if (!key.empty()) (*query)[key] = value;
        if (amp == std::string_view::npos) break;
        rest = rest.substr(amp + 1);
    }
}

void ParseHeaders(std::string_view head, std::map<std::string, std::string> *headers) {
    size_t pos = head.find("\r\n");
    while (pos != std::string_view::npos) {
        pos += 2;
        size_t end = head.find("\r\n", pos);
        std::string_view line = head.substr(pos, end == std::string_view::npos ? std::string_view::npos : end - pos);
        pos = end;
        size_t colon = line.find(':');
        if (colon == std::string_view::npos || colon == 0) continue;
        std::string name(line.substr(0, colon));
        for (char &c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        std::string_view value = line.substr(colon + 1);
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.remove_prefix(1);
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) value.remove_suffix(1);
        auto [it, inserted] = headers->emplace(name, value);
        if (!inserted) it->second.append(", ").append(value);
    }
}

std::string Header(const Request &req, const std::string &name) {
    auto it = req.headers.find(name);
    return it == req.headers.end() ? "" : it->second;
}

bool IsLoopback(const std::string &address) {
    in_addr addr{};
    return inet_pton(AF_INET, address.c_str(), &addr) == 1 && (ntohl(addr.s_addr) >> 24) == 127;
}

namespace {
bool ValidPortSuffix(std::string_view suffix) {
    if (suffix.empty()) return true;
    if (suffix.front() != ':') return false;
    suffix.remove_prefix(1);
    if (suffix.empty() || suffix.size() > 5) return false;
    unsigned value = 0;
    for (char c : suffix) {
        if (c < '0' || c > '9') return false;
        value = value * 10 + unsigned(c - '0');
    }
    return value > 0 && value <= 65535;
}
}

bool IsAllowedHost(std::string_view host) {
    if (host.empty()) return false;
    if (host.front() == '[') {
        size_t close = host.find(']');
        if (close == std::string_view::npos) return false;
        if (!ValidPortSuffix(host.substr(close + 1))) return false;
        in6_addr addr6{};
        return inet_pton(AF_INET6, std::string(host.substr(1, close - 1)).c_str(), &addr6) == 1;
    }
    size_t colon = host.rfind(':');
    std::string_view name = colon == std::string_view::npos ? host : host.substr(0, colon);
    if (colon != std::string_view::npos && !ValidPortSuffix(host.substr(colon))) return false;
    if (name == "localhost") return true;
    in_addr addr4{};
    return inet_pton(AF_INET, std::string(name).c_str(), &addr4) == 1;
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
    return path_matched ? Error(405, "method_not_allowed") : Error(404, "not_found");
}

}
