#include <cstdio>
#include <cstdlib>

#include "config.hpp"
#include "platform/platform.hpp"
#include "test.hpp"
#include "util/fs.hpp"
#include "util/json.hpp"
#include "util/sha256.hpp"
#include "util/timeutil.hpp"

using namespace openpsapp;

TEST(json_writer_nests_and_escapes) {
    JsonWriter w;
    w.BeginObject()
        .Int("a", 1)
        .BeginArray("b")
        .String("x\"y")
        .Bool(true)
        .Null()
        .EndArray()
        .StringOrNull("c", "")
        .String("d", "line\nbreak\x01")
        .EndObject();
    CHECK_EQ(w.take(), std::string(R"({"a":1,"b":["x\"y",true,null],"c":null,"d":"line\nbreak\u0001"})"));
}

TEST(sha256_matches_known_vectors) {
    CHECK_EQ(Sha256::Hex(""), std::string("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    CHECK_EQ(Sha256::Hex("abc"), std::string("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
    std::string million(1000000, 'a');
    CHECK_EQ(Sha256::Hex(million), std::string("cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"));
}

TEST(timestamps_normalize_to_utc) {
    CHECK_EQ(timeutil::NormalizeTimestamp("2026-11-07 20:15:00"), std::string("2026-11-07T20:15:00Z"));
    CHECK_EQ(timeutil::NormalizeTimestamp("2026-10-01 10:00:00.123"), std::string("2026-10-01T10:00:00Z"));
    CHECK_EQ(timeutil::NormalizeTimestamp("2026-10-01T10:00:00Z"), std::string("2026-10-01T10:00:00Z"));
    CHECK_EQ(timeutil::NormalizeTimestamp("1762546500"), std::string("2025-11-07T20:15:00Z"));
    CHECK_EQ(timeutil::NormalizeTimestamp("1762546500000"), std::string("2025-11-07T20:15:00Z"));
    CHECK_EQ(timeutil::NormalizeTimestamp(""), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp("0"), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp("garbage"), std::string(""));
    CHECK_EQ(timeutil::FormatIsoOrEmpty(0), std::string(""));
}

TEST(timestamps_reject_overflowing_and_malformed_input) {
    CHECK_EQ(timeutil::NormalizeTimestamp("9223372036854775808"), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp("12345678901234567890"), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp(std::string(64, '9')), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp("-1762546500"), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp("1762546500x"), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp(" 1762546500"), std::string(""));
    std::string with_nul = "1762546500";
    with_nul[5] = '\0';
    CHECK_EQ(timeutil::NormalizeTimestamp(with_nul), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp("2026-11-07"), std::string(""));
    CHECK_EQ(timeutil::NormalizeTimestamp("0000-00-00 00:00:00"), std::string(""));
}

TEST(read_range_reads_slices_and_rejects_overruns) {
    std::string path = platform::Root() + "/data/range.bin";
    fs::EnsureDir(platform::Root() + "/data");
    CHECK(fs::WriteFileAtomic(path, "0123456789"));

    std::string out;
    CHECK(fs::ReadRange(path, 2, 3, &out));
    CHECK_EQ(out, std::string("234"));
    CHECK(fs::ReadRange(path, 7, -1, &out));
    CHECK_EQ(out, std::string("789"));
    CHECK(fs::ReadFile(path, &out));
    CHECK_EQ(out, std::string("0123456789"));

    CHECK(!fs::ReadRange(path, 8, 5, &out));
    CHECK(!fs::ReadRange(path, 0, -1, &out, 4));
    CHECK(!fs::ReadRange(path + ".missing", 0, -1, &out));
}

TEST(config_language_follows_the_console_when_auto) {
    setenv("OPENPSAPP_LANG_ID", "17", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("pt-BR"));
    CHECK_EQ(ResolveLanguage(""), std::string("pt-BR"));
    CHECK_EQ(ResolveLanguage("ja-JP"), std::string("ja-JP"));
    setenv("OPENPSAPP_LANG_ID", "20", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("es-419"));
    setenv("OPENPSAPP_LANG_ID", "999", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("en-US"));
    setenv("OPENPSAPP_LANG_ID", "en", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("en-US"));
    unsetenv("OPENPSAPP_LANG_ID");
    CHECK_EQ(ResolveLanguage("auto"), std::string("en-US"));
}

TEST(config_reads_and_writes_the_cloud_server) {
    std::string dir = platform::Root() + "/data";
    CHECK(fs::EnsureDir(dir));
    std::string path = dir + "/config-cloud-test.json";
    std::remove(path.c_str());
    std::string warning;
    CHECK_EQ(LoadOrCreateConfig(path, &warning).server, std::string(""));
    std::string text;
    CHECK(fs::ReadFile(path, &text, 4096));
    CHECK(text.find("\"server\":\"\"") != std::string::npos);
    CHECK_EQ(CloudServer(LoadOrCreateConfig(path, &warning)), std::string("https://openpsapp.com"));
    CHECK(fs::WriteFileAtomic(path, "{\"port\":7777,\"server\":\"https://api.example.test\"}"));
    CHECK_EQ(LoadOrCreateConfig(path, &warning).server, std::string("https://api.example.test"));
    CHECK_EQ(CloudServer(LoadOrCreateConfig(path, &warning)), std::string("https://api.example.test"));
    std::remove(path.c_str());
}
