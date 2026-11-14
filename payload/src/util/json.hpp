#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct cJSON;

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

    JsonWriter &BeginObject(std::string_view key);
    JsonWriter &BeginArray(std::string_view key);
    JsonWriter &String(std::string_view key, std::string_view value);
    JsonWriter &Int(std::string_view key, int64_t value);
    JsonWriter &Bool(std::string_view key, bool value);
    JsonWriter &Null(std::string_view key);
    JsonWriter &StringOrNull(std::string_view key, std::string_view value);

    std::string take() { return std::move(out_); }

private:
    void BeforeValue();
    void Escape(std::string_view value);

    std::string out_;
    std::vector<bool> has_items_;
    bool after_key_ = false;
};

std::string JsonString(const cJSON *object, const char *key);

}
