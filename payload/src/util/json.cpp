#include "util/json.hpp"

#include <cstdio>

namespace playtrack {
void JsonWriter::BeforeValue() {
    if (after_key_) {
        after_key_ = false;
        return;
    }
    if (!has_items_.empty()) {
        if (has_items_.back()) out_ += ',';
        has_items_.back() = true;
    }
}

void JsonWriter::Escape(std::string_view value) {
    out_ += '"';
    for (unsigned char c : value) {
        switch (c) {
        case '"': out_ += "\\\""; break;
        case '\\': out_ += "\\\\"; break;
        case '\n': out_ += "\\n"; break;
        case '\r': out_ += "\\r"; break;
        case '\t': out_ += "\\t"; break;
        default:
            if (c < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out_ += buf;
            } else {
                out_ += static_cast<char>(c);
            }
        }
    }
    out_ += '"';
}

JsonWriter &JsonWriter::BeginObject() {
    BeforeValue();
    out_ += '{';
    has_items_.push_back(false);
    return *this;
}

JsonWriter &JsonWriter::EndObject() {
    out_ += '}';
    has_items_.pop_back();
    return *this;
}

JsonWriter &JsonWriter::BeginArray() {
    BeforeValue();
    out_ += '[';
    has_items_.push_back(false);
    return *this;
}

JsonWriter &JsonWriter::EndArray() {
    out_ += ']';
    has_items_.pop_back();
    return *this;
}

JsonWriter &JsonWriter::Key(std::string_view key) {
    BeforeValue();
    Escape(key);
    out_ += ':';
    after_key_ = true;
    return *this;
}

JsonWriter &JsonWriter::String(std::string_view value) {
    BeforeValue();
    Escape(value);
    return *this;
}

JsonWriter &JsonWriter::Int(int64_t value) {
    BeforeValue();
    out_ += std::to_string(value);
    return *this;
}

JsonWriter &JsonWriter::Bool(bool value) {
    BeforeValue();
    out_ += value ? "true" : "false";
    return *this;
}

JsonWriter &JsonWriter::Null() {
    BeforeValue();
    out_ += "null";
    return *this;
}

JsonWriter &JsonWriter::StringOrNull(std::string_view value) {
    return value.empty() ? Null() : String(value);
}

}
