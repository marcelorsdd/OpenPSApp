#include <cstdlib>

#include "config.hpp"
#include "platform/platform.hpp"
#include "test.hpp"
#include "util/fs.hpp"
#include "util/json.hpp"
#include "util/sha256.hpp"
#include "util/timeutil.hpp"

using namespace playtrack;

TEST(json_writer_nests_and_escapes) {
    JsonWriter w;
    w.BeginObject()
        .Key("a").Int(1)
        .Key("b").BeginArray().String("x\"y").Bool(true).Null().EndArray()
        .Key("c").StringOrNull("")
        .Key("d").String("line\nbreak\x01")
        .EndObject();
    CHECK_EQ(w.str(), std::string(R"({"a":1,"b":["x\"y",true,null],"c":null,"d":"line\nbreak\u0001"})"));
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
    setenv("PLAYTRACK_LANG_ID", "17", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("pt-BR"));
    CHECK_EQ(ResolveLanguage(""), std::string("pt-BR"));
    CHECK_EQ(ResolveLanguage("ja-JP"), std::string("ja-JP"));
    setenv("PLAYTRACK_LANG_ID", "20", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("es-419"));
    setenv("PLAYTRACK_LANG_ID", "999", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("en-US"));
    setenv("PLAYTRACK_LANG_ID", "en", 1);
    CHECK_EQ(ResolveLanguage("auto"), std::string("en-US"));
    unsetenv("PLAYTRACK_LANG_ID");
    CHECK_EQ(ResolveLanguage("auto"), std::string("en-US"));
}
