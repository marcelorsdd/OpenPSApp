#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace openpsapp {
class JsonWriter {
public:
    JsonWriter &BeginObject();
    JsonWriter &EndObject();
    JsonWriter &BeginArray();
    JsonWriter &EndArray();
    JsonWriter &Key(std::string_view key);
    JsonWriter &String(std::string_view value);
    JsonWriter &Int(int64_t value);
    JsonWriter &Bool(bool value);
    JsonWriter &Null();

    JsonWriter &StringOrNull(std::string_view value);

    const std::string &str() const { return out_; }
    std::string take() { return std::move(out_); }

private:
    void BeforeValue();
    void Escape(std::string_view value);

    std::string out_;
    std::vector<bool> has_items_;
    bool after_key_ = false;
};

}
