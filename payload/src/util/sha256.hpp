#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace playtrack {
class Sha256 {
public:
    Sha256();
    void Update(const void *data, size_t size);
    std::array<uint8_t, 32> Digest();
    std::string HexDigest();

    static std::string Hex(std::string_view data);

private:
    void Block(const uint8_t *block);

    uint32_t state_[8];
    uint8_t buffer_[64];
    size_t buffered_ = 0;
    uint64_t total_ = 0;
};

}
