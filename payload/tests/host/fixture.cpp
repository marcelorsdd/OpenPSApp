#include "fixture.hpp"

#include <cstdio>

#include "sqlite3.h"
#include "util/fs.hpp"
#include "util/sha256.hpp"

namespace fixture {
namespace {
constexpr const char *kSchema = R"SQL(
CREATE TABLE tbl_contentinfo (titleId TEXT, titleName TEXT, is3rdPartyVideoApp INTEGER, categoryType INTEGER, metaDataPath TEXT);
CREATE TABLE tbl_appbrowse (titleId TEXT, titleName TEXT);
CREATE TABLE tbl_iconinfo_0515316669 (titleId TEXT, playedTimeOnConsole INTEGER, lastAccessTime TEXT, lastPlayedDateOnConsole TEXT);
CREATE TABLE tbl_iconinfo_0000043690 (titleId TEXT, playedTimeOnConsole INTEGER, lastAccessTime TEXT, lastPlayedDateOnConsole TEXT);

INSERT INTO tbl_contentinfo VALUES
  ('PPSA01234', 'Astro Test', 0, 0, '/mnt/ext0/user/app/PPSA01234/sce_sys'),
  ('CUSA00001', 'Old Game', 0, 0, NULL),
  ('PPSA01650', 'YouTube', 1, 65536, '/user/app/PPSA01650/sce_sys'),
  ('PPSA09999', 'Never Played', 0, 0, NULL),
  ('PMGR09999', 'Prospero Manager', 0, 65536, NULL),
  ('PPSA05555', '', 0, 0, NULL),
  ('PPSA07777', 'Other Profile Game', 0, 0, NULL);
INSERT INTO tbl_appbrowse VALUES ('PPSA05555', 'Browse Name');

INSERT INTO tbl_iconinfo_0515316669 VALUES
  ('PPSA01234', 7200, '2026-11-07 22:00:00.500', '2026-11-07 20:15:00.123'),
  ('CUSA00001', 3600, '2026-10-01 10:00:00.123', '2026-10-01 10:00:00.200'),
  ('PPSA01650', 9000, '2026-11-08 08:00:00', '2026-11-08 08:00:00'),
  ('PPSA09999', NULL, '2026-10-20 12:00:44.583', NULL),
  ('PMGR09999', 50, '2026-11-08 09:00:00', '2026-11-08 09:00:00'),
  ('PPSA05555', 60, NULL, NULL);
INSERT INTO tbl_iconinfo_0000043690 VALUES
  ('PPSA01234', 99999, '2026-11-08 07:00:00', '2026-11-08 07:00:00'),
  ('PPSA07777', 500, '2026-09-09 09:09:09', '2026-09-09 09:09:09');
)SQL";

bool WriteFile(const std::string &path, const std::string &data) {
    size_t slash = path.rfind('/');
    return playtrack::fs::EnsureDir(path.substr(0, slash)) && playtrack::fs::WriteFileAtomic(path, data);
}

void PutBe16(std::string &b, size_t off, uint16_t v) {
    b[off] = static_cast<char>(v >> 8);
    b[off + 1] = static_cast<char>(v);
}
void PutBe32(std::string &b, size_t off, uint32_t v) {
    for (int i = 0; i < 4; ++i) b[off + i] = static_cast<char>(v >> (24 - 8 * i));
}
void PutBe64(std::string &b, size_t off, uint64_t v) {
    PutBe32(b, off, static_cast<uint32_t>(v >> 32));
    PutBe32(b, off + 4, static_cast<uint32_t>(v));
}

uint64_t TrophyTicks(int64_t unix_seconds) {
    return unix_seconds ? static_cast<uint64_t>(unix_seconds + 62135596800LL) * 1000000ULL : 0;
}

std::string Png(const std::string &tag) { return std::string("\x89PNG\r\n\x1a\n", 8) + tag; }

constexpr const char *kTropConf = R"JSON({
  "schemaVersion": "1.00", "trophyNpCommId": "NPWR00001_00", "defaultLanguage": "en-US",
  "languages": ["en-US", "pt-BR"],
  "trophies": [
    {"id": "0000", "hidden": false, "grade": "P", "hasReward": false},
    {"id": "0001", "hidden": false, "grade": "G", "hasReward": false},
    {"id": "0002", "hidden": true, "grade": "S", "hasReward": false},
    {"id": "0003", "hidden": false, "grade": "B", "hasReward": false}
  ]
})JSON";

std::string TropMeta(const char *title, const char *const names[4]) {
    std::string out = std::string(R"({"schemaVersion":"0.90","trophyNpCommId":"NPWR00001_00","metadata":{"titleMetadata":{"name":")") +
                      title + R"("},"trophyMetadata":[)";
    for (int i = 0; i < 4; ++i) {
        char item[160];
        std::snprintf(item, sizeof(item), R"(%s{"id":"%04d","name":"%s","detail":"%s detail"})", i ? "," : "", i, names[i],
                      names[i]);
        out += item;
    }
    return out + "]}}";
}
}

std::string MakeUcp(const std::vector<std::pair<std::string, std::string>> &entries) {
    size_t table_end = 0x60 + entries.size() * 64;
    std::string out(table_end, '\0');
    PutBe32(out, 0, 0xb228c60a);
    PutBe32(out, 4, 1);
    PutBe32(out, 0x10, static_cast<uint32_t>(entries.size()));
    PutBe32(out, 0x14, 64);
    for (size_t i = 0; i < entries.size(); ++i) {
        size_t row = 0x60 + i * 64;
        out.replace(row, entries[i].first.size(), entries[i].first);
        PutBe64(out, row + 32, out.size());
        PutBe64(out, row + 40, entries[i].second.size());
        out += entries[i].second;
    }
    PutBe64(out, 8, out.size());
    return out;
}

std::string HexBytes(const std::string &hex) {
    std::string out;
    for (size_t i = 0; i + 1 < hex.size(); i += 2) out += static_cast<char>(std::stoi(hex.substr(i, 2), nullptr, 16));
    return out;
}

std::string MakeT2pd(const std::vector<StateSpec> &states) {
    constexpr uint32_t kOwnerPayload = 0x60;
    constexpr uint32_t kPayload = 0x50;
    std::string stream(48 + 2 * 32, '\0');
    stream.replace(0, 4, "T2TD");
    PutBe32(stream, 4, 0x10000);
    PutBe32(stream, 8, 2);

    PutBe32(stream, 48, 0x2);
    PutBe32(stream, 52, kOwnerPayload);
    PutBe32(stream, 56, 1);
    PutBe32(stream, 60, 1);
    PutBe64(stream, 64, stream.size());
    std::string owner(16 + kOwnerPayload, '\0');
    PutBe32(owner, 0, 0x2);
    PutBe32(owner, 4, kOwnerPayload);
    owner.replace(16 + 0x20, 8, HexBytes(kAccountIdHex));
    PutBe32(owner, 16 + 0x28, static_cast<uint32_t>(kUserId));
    owner.replace(16 + 0x30, 16, HexBytes(kDeviceIdHex));
    stream += owner;

    PutBe32(stream, 80, 0x800);
    PutBe32(stream, 84, kPayload);
    PutBe32(stream, 88, 1);
    PutBe32(stream, 92, static_cast<uint32_t>(states.size()));
    PutBe64(stream, 96, stream.size());
    for (const StateSpec &s : states) {
        std::string rec(16 + kPayload, '\0');
        PutBe32(rec, 0, 0x800);
        PutBe32(rec, 4, kPayload);
        PutBe32(rec, 0x10, static_cast<uint32_t>(s.id));
        PutBe32(rec, 0x14, s.flags);
        PutBe64(rec, 0x20, TrophyTicks(s.earned_unix));
        PutBe64(rec, 0x30, TrophyTicks(s.second_unix));
        stream += rec;
    }
    stream.resize((stream.size() + 1023) / 1024 * 1024, '\0');

    size_t pages = stream.size() / 1024;
    std::string out(64, '\0');
    out.replace(0, 4, "T2PD");
    PutBe32(out, 4, 0x10000);
    PutBe64(out, 8, 1056);
    PutBe64(out, 32, pages);
    for (size_t p = 0; p < pages; ++p) {
        std::string page = stream.substr(p * 1024, 1024);
        playtrack::Sha256 sha;
        sha.Update(page.data(), page.size());
        auto digest = sha.Digest();
        out += page;
        out.append(reinterpret_cast<const char *>(digest.data()), digest.size());
    }
    return out;
}

std::string MakeNpbind(const std::string &np_comm_id) {
    std::string out(0x80, '\0');
    PutBe32(out, 0, 0xd294a018);
    PutBe32(out, 4, 1);
    auto tlv = [&out](uint16_t tag, const std::string &value) {
        std::string rec(4, '\0');
        PutBe16(rec, 0, tag);
        PutBe16(rec, 2, static_cast<uint16_t>(value.size()));
        out += rec + value;
    };
    tlv(0x0010, np_comm_id);
    tlv(0x0011, std::string("0") + std::string(11, '\0'));
    tlv(0x0012, std::string(16, '\x5a'));
    return out;
}

bool Build(const std::string &root, std::string *err) {
    std::string mms = root + "/system_data/priv/mms";
    if (!playtrack::fs::EnsureDir(mms)) {
        *err = "mkdir " + mms;
        return false;
    }
    std::string db_path = mms + "/app.db";
    std::remove(db_path.c_str());

    sqlite3 *db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        *err = "open " + db_path;
        sqlite3_close(db);
        return false;
    }
    char *msg = nullptr;
    bool ok = sqlite3_exec(db, kSchema, nullptr, nullptr, &msg) == SQLITE_OK;
    if (!ok) *err = msg ? msg : "sqlite exec failed";
    sqlite3_free(msg);
    sqlite3_close(db);
    if (!ok) return false;

    static const char *const kEnNames[4] = {"Platinum Run", "Gold Rush", "Secret Silver", "First Steps"};
    static const char *const kPtNames[4] = {"Corrida Platina", "Corrida do Ouro", "Prata Secreta", "Primeiros Passos"};
    std::string ucp = MakeUcp({
        {"icon0_en-US.png", Png("set-icon")},
        {"trop0000.png", Png("trop0000")},
        {"trop0001.png", Png("trop0001")},
        {"trop0002.png", Png("trop0002")},
        {"trop0003.png", Png("trop0003")},
        {"tropconf.json", kTropConf},
        {"tropmeta_en-US.json", TropMeta("Astro Test", kEnNames)},
        {"tropmeta_pt-BR.json", TropMeta("Astro Teste", kPtNames)},
    });
    std::string dat = MakeT2pd({
        {0, 0x00, 0, 0},
        {1, 0x11, 1762545600, 1762545600},
        {2, 0x10, 0, 1762546000},
        {3, 0x11, 1761991200, 1761991200},
    });
    std::string corrupt = dat;
    corrupt[64 + 100] ^= 0x01;

    std::string home = root + "/user/home/" + std::string(kUserHex) + "/trophy2/nobackup/data/";
    std::string conf = root + "/user/trophy2/nobackup/conf/";
    ok = WriteFile(root + "/user/appmeta/PPSA01234/icon0.png", Png("astro")) &&
         WriteFile(root + "/user/appmeta/PPSA01234/pic0.png", Png("astro-hero")) &&
         WriteFile(root + "/user/appmeta/external/CUSA00001/icon0.png", Png("old")) &&
         WriteFile(conf + kSetId + "/TROPHY.UCP", ucp) && WriteFile(home + kSetId + "/TRPTITLE.DAT", dat) &&
         WriteFile(conf + kCorruptSetId + "/TROPHY.UCP", ucp) && WriteFile(home + kCorruptSetId + "/TRPTITLE.DAT", corrupt) &&
         WriteFile(root + "/mnt/ext0/user/app/PPSA01234/sce_sys/trophy2/npbind.dat", MakeNpbind(kSetId)) &&
         WriteFile(root + "/user/app/PPSA05555/sce_sys/trophy2/npbind.dat", MakeNpbind(kCorruptSetId));
    if (!ok) *err = "writing fixture files under " + root;
    return ok;
}

}
