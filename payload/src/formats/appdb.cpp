#include "formats/appdb.hpp"

#include "sqlite3.h"

namespace playtrack {
std::string AppDb::Row::Text(int col) const {
    const unsigned char *text = sqlite3_column_text(stmt_, col);
    return text ? reinterpret_cast<const char *>(text) : "";
}

int64_t AppDb::Row::Int(int col) const { return sqlite3_column_int64(stmt_, col); }

AppDb::~AppDb() {
    if (db_) sqlite3_close(db_);
}

bool AppDb::Open(const std::string &path, std::string *err) {
    int rc = sqlite3_open_v2(path.c_str(), &db_, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, nullptr);
    if (rc != SQLITE_OK) {
        if (err) *err = path + ": " + (db_ ? sqlite3_errmsg(db_) : sqlite3_errstr(rc));
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    sqlite3_busy_timeout(db_, 2000);
    return true;
}

bool AppDb::IsSafeIdentifier(const std::string &name) {
    if (name.empty()) return false;
    for (char c : name) {
        bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
        if (!ok) return false;
    }
    return true;
}

bool AppDb::ForEachRow(const std::string &sql, const std::vector<std::string> &args,
                       const std::function<void(const Row &)> &fn) const {
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    for (size_t i = 0; i < args.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), args[i].c_str(), -1, SQLITE_TRANSIENT);
    }
    Row row(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) fn(row);
    sqlite3_finalize(stmt);
    return true;
}

std::vector<std::string> AppDb::Tables(const std::string &like) const {
    std::vector<std::string> out;
    ForEachRow("SELECT name FROM sqlite_master WHERE type='table' AND name LIKE ?1 ORDER BY name", {like},
               [&out](const Row &r) { out.push_back(r.Text(0)); });
    return out;
}

std::vector<std::string> AppDb::Columns(const std::string &table) const {
    std::vector<std::string> out;
    if (!IsSafeIdentifier(table)) return out;
    ForEachRow("PRAGMA table_info(" + table + ")", {}, [&out](const Row &r) { out.push_back(r.Text(1)); });
    return out;
}

int64_t AppDb::RowCount(const std::string &table) const {
    int64_t count = -1;
    if (!IsSafeIdentifier(table)) return count;
    ForEachRow("SELECT COUNT(*) FROM " + table, {}, [&count](const Row &r) { count = r.Int(0); });
    return count;
}

std::vector<std::vector<std::pair<std::string, std::string>>> AppDb::Sample(const std::string &table, int limit) const {
    std::vector<std::vector<std::pair<std::string, std::string>>> rows;
    if (!IsSafeIdentifier(table)) return rows;
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT * FROM " + table + " LIMIT " + std::to_string(limit);
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return rows;
    Row row(stmt);
    int cols = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::pair<std::string, std::string>> values;
        for (int i = 0; i < cols; ++i) {
            std::string value = sqlite3_column_type(stmt, i) == SQLITE_BLOB
                                    ? "<blob " + std::to_string(sqlite3_column_bytes(stmt, i)) + " bytes>"
                                    : row.Text(i);
            values.emplace_back(sqlite3_column_name(stmt, i), std::move(value));
        }
        rows.push_back(std::move(values));
    }
    sqlite3_finalize(stmt);
    return rows;
}

bool AppDb::BackupTo(const std::string &path, std::string *err) const {
    sqlite3 *dest = nullptr;
    if (sqlite3_open(path.c_str(), &dest) != SQLITE_OK) {
        if (err) *err = "open " + path + ": " + sqlite3_errmsg(dest);
        sqlite3_close(dest);
        return false;
    }
    bool ok = false;
    if (sqlite3_backup *backup = sqlite3_backup_init(dest, "main", db_, "main")) {
        ok = sqlite3_backup_step(backup, -1) == SQLITE_DONE;
        sqlite3_backup_finish(backup);
    }
    if (!ok && err) *err = std::string("backup: ") + sqlite3_errmsg(dest);
    sqlite3_close(dest);
    return ok;
}

}
