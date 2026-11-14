#include "net/http_message.hpp"

#include <cctype>
#include <cstdlib>

namespace openpsapp::net {
namespace {
std::string Lower(std::string_view s) {
    std::string out(s);
    for (char &c : out) c = char(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

std::string_view Trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
    return s;
}

bool DecodeChunked(std::string_view data, std::string *body, std::string *err) {
    body->clear();
    size_t pos = 0;
    while (true) {
        size_t line_end = data.find("\r\n", pos);
        if (line_end == std::string_view::npos) {
            *err = "truncated chunk size";
            return false;
        }
        std::string size_text(data.substr(pos, data.find_first_of(";\r", pos) - pos));
        char *end = nullptr;
        unsigned long size = std::strtoul(size_text.c_str(), &end, 16);
        if (size_text.empty() || *end != '\0') {
            *err = "bad chunk size";
            return false;
        }
        pos = line_end + 2;
        if (size == 0) return true;
        if (data.size() < pos + size + 2) {
            *err = "truncated chunk";
            return false;
        }
        body->append(data.substr(pos, size));
        pos += size + 2;
    }
}
}

std::string BuildRequest(std::string_view method, const Url &url, const Headers &headers, std::string_view body) {
    std::string out;
    out.append(method).append(" ").append(url.target).append(" HTTP/1.1\r\n");
    out.append("Host: ").append(url.host);
    if (url.port != 443) out.append(":").append(std::to_string(url.port));
    out.append("\r\nConnection: close\r\n");
    for (const auto &[name, value] : headers) out.append(name).append(": ").append(value).append("\r\n");
    if (!body.empty()) out.append("Content-Length: ").append(std::to_string(body.size())).append("\r\n");
    out.append("\r\n").append(body);
    return out;
}

bool ParseResponse(std::string_view raw, int *status, std::string *body, std::string *err) {
    size_t head_end = raw.find("\r\n\r\n");
    if (head_end == std::string_view::npos) {
        *err = "incomplete response headers";
        return false;
    }
    std::string_view head = raw.substr(0, head_end);
    std::string_view data = raw.substr(head_end + 4);
    size_t line_end = head.find("\r\n");
    std::string_view status_line = head.substr(0, line_end);
    if (status_line.substr(0, 5) != "HTTP/" || status_line.size() < 12 || status_line[8] != ' ') {
        *err = "bad status line";
        return false;
    }
    std::string code(status_line.substr(9, 3));
    if (!std::isdigit(static_cast<unsigned char>(code[0])) || !std::isdigit(static_cast<unsigned char>(code[1])) ||
        !std::isdigit(static_cast<unsigned char>(code[2]))) {
        *err = "bad status code";
        return false;
    }
    *status = std::atoi(code.c_str());

    bool chunked = false;
    long long length = -1;
    size_t pos = line_end == std::string_view::npos ? head.size() : line_end + 2;
    while (pos < head.size()) {
        size_t next = head.find("\r\n", pos);
        std::string_view line = head.substr(pos, next == std::string_view::npos ? std::string_view::npos : next - pos);
        pos = next == std::string_view::npos ? head.size() : next + 2;
        size_t colon = line.find(':');
        if (colon == std::string_view::npos) continue;
        std::string name = Lower(Trim(line.substr(0, colon)));
        std::string value = Lower(Trim(line.substr(colon + 1)));
        if (name == "transfer-encoding" && value.find("chunked") != std::string::npos) chunked = true;
        if (name == "content-length") length = std::atoll(value.c_str());
    }

    if (*status == 204 || *status == 304 || (*status >= 100 && *status < 200)) {
        body->clear();
        return true;
    }
    if (chunked) return DecodeChunked(data, body, err);
    if (length >= 0) {
        if (data.size() < size_t(length)) {
            *err = "truncated body";
            return false;
        }
        body->assign(data.substr(0, size_t(length)));
        return true;
    }
    body->assign(data);
    return true;
}

}
