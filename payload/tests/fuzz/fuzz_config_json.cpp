#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

#include "config.hpp"

namespace {
std::string WriteTemp(const uint8_t *data, size_t size) {
    char path[] = "/tmp/opsa_fuzz_cfg_XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) return "";
    if (size) {
        ssize_t n = write(fd, data, size);
        (void)n;
    }
    close(fd);
    return path;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string path = WriteTemp(data, size);
    if (path.empty()) return 0;
    std::string warning;
    openpsapp::Config cfg = openpsapp::LoadOrCreateConfig(path, &warning);
    volatile int sink = cfg.port + static_cast<int>(cfg.language.size()) + static_cast<int>(cfg.server.size());
    (void)sink;
    unlink(path.c_str());
    return 0;
}
