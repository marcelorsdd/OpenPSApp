#include "util/log.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cstdarg>
#include <cstdio>
#include <deque>
#include <mutex>

#include "util/fs.hpp"
#include "util/timeutil.hpp"

namespace playtrack::log {
namespace {
constexpr size_t kRingSize = 500;
constexpr int64_t kMaxFileBytes = 1 << 20;

std::mutex g_mu;
std::deque<std::string> g_ring;
int g_fd = -1;

void Write(const char *level, bool durable, const char *fmt, va_list args) {
    char msg[1024];
    std::vsnprintf(msg, sizeof(msg), fmt, args);
    std::string line = timeutil::NowIso() + " " + level + " " + msg;

    std::lock_guard<std::mutex> lock(g_mu);
    std::printf("[playtrack] %s\n", line.c_str());
    std::fflush(stdout);
    g_ring.push_back(line);
    if (g_ring.size() > kRingSize) g_ring.pop_front();
    if (g_fd >= 0) {
        line += '\n';
        if (write(g_fd, line.data(), line.size()) > 0 && durable) fsync(g_fd);
    }
}
}

void Init(const std::string &file_path) {
    std::lock_guard<std::mutex> lock(g_mu);
    if (g_fd >= 0) close(g_fd);
    int flags = O_WRONLY | O_CREAT | O_APPEND;
    auto st = fs::StatPath(file_path);
    if (st && st->size > kMaxFileBytes) flags |= O_TRUNC;
    g_fd = open(file_path.c_str(), flags, 0644);
}

void Info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Write("INFO", false, fmt, args);
    va_end(args);
}

void Error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Write("ERROR", true, fmt, args);
    va_end(args);
}

int RawFd() { return g_fd; }

std::vector<std::string> Tail(size_t lines) {
    std::lock_guard<std::mutex> lock(g_mu);
    size_t start = g_ring.size() > lines ? g_ring.size() - lines : 0;
    return {g_ring.begin() + static_cast<long>(start), g_ring.end()};
}

}
