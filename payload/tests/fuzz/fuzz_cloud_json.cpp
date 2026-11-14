#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "cloud/store.hpp"

namespace {
std::string WriteTemp(const uint8_t *data, size_t size) {
    char path[] = "/tmp/opsa_fuzz_cloud_XXXXXX";
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
    {
        openpsapp::cloud::CloudStore store(path);
        auto link = store.Get("user");
        if (link) {
            volatile size_t sink = link->token.size() + link->device_id.size() + link->profile_name.size();
            (void)sink;
        }
        for (const auto &token : store.Revocations()) {
            volatile size_t sink = token.size();
            (void)sink;
        }
    }
    unlink(path.c_str());
    return 0;
}
