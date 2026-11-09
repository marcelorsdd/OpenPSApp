#include "config.hpp"

#include "cJSON.h"
#include "platform/platform.hpp"
#include "util/fs.hpp"
#include "util/json.hpp"

namespace playtrack {
namespace {
constexpr const char *kSystemLanguages[] = {
    "ja-JP", "en-US", "fr-FR", "es-ES", "de-DE", "it-IT", "nl-NL", "pt-PT", "ru-RU", "ko-KR", "zh-Hant",
    "zh-Hans", "fi-FI", "sv-SE", "da-DK", "no-NO", "pl-PL", "pt-BR", "en-GB", "tr-TR", "es-419", "ar-AE",
    "fr-CA", "cs-CZ", "hu-HU", "el-GR", "ro-RO", "th-TH", "vi-VN", "id-ID", "uk-UA",
};

std::string Serialize(const Config &cfg) {
    JsonWriter w;
    w.BeginObject()
        .Key("port").Int(cfg.port)
        .Key("language").String(cfg.language)
        .Key("debug").Bool(cfg.debug)
        .EndObject();
    return w.take() + "\n";
}
}

Config LoadOrCreateConfig(const std::string &path, std::string *warning) {
    Config cfg;
    std::string text;
    if (!fs::ReadFile(path, &text, 64 * 1024)) {
        if (!fs::WriteFileAtomic(path, Serialize(cfg)) && warning) *warning = "could not write default " + path;
        return cfg;
    }

    cJSON *root = cJSON_ParseWithLength(text.data(), text.size());
    if (!cJSON_IsObject(root)) {
        if (warning) *warning = path + " is not a JSON object; using defaults";
        cJSON_Delete(root);
        return cfg;
    }

    if (const cJSON *v = cJSON_GetObjectItemCaseSensitive(root, "port"); cJSON_IsNumber(v)) {
        if (v->valueint > 0 && v->valueint < 65536) cfg.port = v->valueint;
        else if (warning) *warning = "port out of range ignored";
    }
    if (const cJSON *v = cJSON_GetObjectItemCaseSensitive(root, "language"); cJSON_IsString(v) && *v->valuestring) {
        cfg.language = v->valuestring;
    }
    if (const cJSON *v = cJSON_GetObjectItemCaseSensitive(root, "debug"); cJSON_IsBool(v)) {
        cfg.debug = cJSON_IsTrue(v);
    }
    cJSON_Delete(root);
    return cfg;
}

std::string ResolveLanguage(const std::string &configured) {
    if (!configured.empty() && configured != "auto") return configured;
    int id = platform::SystemLanguageId();
    constexpr int kCount = sizeof(kSystemLanguages) / sizeof(kSystemLanguages[0]);
    return id >= 0 && id < kCount ? kSystemLanguages[id] : "en-US";
}

}
