#include "fixture.hpp"
#include "formats/npbind.hpp"
#include "formats/t2pd.hpp"
#include "formats/ucp.hpp"
#include "platform/platform.hpp"
#include "test.hpp"

using namespace playtrack;

TEST(ucp_indexes_entries_and_reads_them_by_offset) {
    std::string path = platform::Root() + "/user/trophy2/nobackup/conf/NPWR00001_00/TROPHY.UCP";
    Ucp ucp;
    std::string err;
    CHECK(ucp.Open(path, &err));
    CHECK_EQ(err, std::string(""));
    CHECK_EQ(ucp.entries().size(), size_t(8));
    const Ucp::Entry *e = ucp.Find("trop0001.png");
    CHECK(e != nullptr);
    std::string data;
    if (e) CHECK(ucp.Read(*e, &data));
    CHECK_EQ(data, std::string("\x89PNG\r\n\x1a\ntrop0001", 16));
    CHECK(ucp.Find("missing.png") == nullptr);
}

TEST(ucp_rejects_bad_magic_and_out_of_bounds_entries) {
    std::string bytes = fixture::MakeUcp({{"a.json", "{}"}});
    Ucp ucp;
    std::string err;
    CHECK(ucp.ParseTable(bytes, bytes.size(), &err));

    std::string bad_magic = bytes;
    bad_magic[0] = 0;
    CHECK(!ucp.ParseTable(bad_magic, bad_magic.size(), &err));

    CHECK(!ucp.ParseTable(bytes, 0x60 + 64, &err));
    CHECK(ucp.entries().empty());
}

TEST(t2pd_reads_states_with_earned_rule) {
    std::string bytes = fixture::MakeT2pd({
        {0, 0x00, 0, 0},
        {1, 0x11, 1762545600, 1762545600},
        {2, 0x10, 0, 1762546000},
        {3, 0x11, 1762545600, 0},
    });
    std::vector<TrophyState> states;
    std::string err;
    CHECK(ParseTrophyStates(bytes, &states, &err));
    CHECK_EQ(err, std::string(""));
    CHECK_EQ(states.size(), size_t(4));
    if (states.size() == 4) {
        CHECK(!states[0].earned);
        CHECK(states[1].earned);
        CHECK_EQ(states[1].earned_at, int64_t(1762545600));
        CHECK(!states[2].earned);
        CHECK_EQ(states[2].earned_at, int64_t(0));
        CHECK(!states[3].earned);
    }
}

TEST(t2pd_rejects_tampered_pages_and_wrong_sizes) {
    std::string bytes = fixture::MakeT2pd({{1, 0x11, 1762545600, 1762545600}});
    std::vector<TrophyState> states;
    std::string err;

    std::string tampered = bytes;
    tampered[64 + 200] ^= 0x01;
    CHECK(!ParseTrophyStates(tampered, &states, &err));
    CHECK(err.find("SHA-256") != std::string::npos);

    CHECK(!ParseTrophyStates(bytes.substr(0, bytes.size() - 1), &states, &err));
    CHECK(!ParseTrophyStates("T2PD", &states, &err));
}

TEST(trophy_time_converts_from_year_one_microseconds) {
    CHECK_EQ(TrophyTimeToUnix(0), int64_t(0));
    CHECK_EQ(TrophyTimeToUnix(62135596800ULL * 1000000ULL), int64_t(0));
    CHECK_EQ(TrophyTimeToUnix((62135596800ULL + 1762545600ULL) * 1000000ULL + 999999), int64_t(1762545600));
}

TEST(npbind_yields_the_trophy_set_id) {
    CHECK_EQ(ParseNpCommId(fixture::MakeNpbind("NPWR57054_00")), std::string("NPWR57054_00"));
    CHECK_EQ(ParseNpCommId(fixture::MakeNpbind("garbage_0000")), std::string(""));
    CHECK_EQ(ParseNpCommId("short"), std::string(""));
    CHECK(IsNpCommId("NPWR00001_00"));
    CHECK(!IsNpCommId("NPWR0001_00"));
    CHECK(!IsNpCommId("XPWR00001_00"));
}

TEST(t2pd_reads_the_owner_record) {
    std::string bytes = fixture::MakeT2pd({{1, 0x11, 1762545600, 1762545600}});
    TrophyOwner owner;
    std::string err;
    CHECK(ParseTrophyOwner(bytes, &owner, &err));
    CHECK_EQ(err, std::string(""));
    CHECK_EQ(owner.account_id_hex, std::string(fixture::kAccountIdHex));
    CHECK_EQ(owner.user_id, static_cast<uint32_t>(fixture::kUserId));
    CHECK_EQ(owner.device_id_hex, std::string(fixture::kDeviceIdHex));

    std::vector<TrophyState> states;
    CHECK(ParseTrophyStates(bytes, &states, &err));
    CHECK_EQ(states.size(), size_t(1));
}
