#include "services/diagnostics.hpp"

#include "formats/appdb.hpp"
#include "formats/t2pd.hpp"
#include "platform/platform.hpp"
#include "services/console_layout.hpp"
#include "util/fs.hpp"

namespace openpsapp::diagnostics {
AppDbReport InspectAppDb() {
    AppDbReport report;
    AppDb db;
    if (!db.Open(platform::ConsolePath(layout::kAppDb), &report.error)) return report;
    report.ok = true;
    report.tables = db.Tables();
    for (const std::string &t : report.tables) {
        bool iconinfo = t.rfind(layout::kIconInfoPrefix, 0) == 0;
        if (!iconinfo && t != "tbl_contentinfo" && t != "tbl_appbrowse") continue;
        report.details.push_back({t, db.RowCount(t), db.Columns(t), db.Sample(t, iconinfo ? 3 : 2)});
    }
    return report;
}

bool CopyAppDb(const std::string &dest, std::string *err) {
    AppDb db;
    return db.Open(platform::ConsolePath(layout::kAppDb), err) && db.BackupTo(dest, err);
}

std::vector<TrophyFileOwner> TrophyFileOwners(const TrophyReport &trophies) {
    std::vector<TrophyFileOwner> files;
    for (const TrophySet &set : trophies.sets) {
        TrophyFileOwner file;
        file.set_id = set.id;
        std::string path = platform::ConsolePath(trophies.data_dir + "/" + set.id + "/TRPTITLE.DAT");
        std::string bytes;
        TrophyOwner owner;
        file.ok = fs::ReadFile(path, &bytes, 16 << 20) && ParseTrophyOwner(bytes, &owner, &file.error);
        file.user_id = owner.user_id;
        file.account_id_hex = owner.account_id_hex;
        file.device_id_hex = owner.device_id_hex;
        files.push_back(std::move(file));
    }
    return files;
}

}
