#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

namespace playtrack {
class AppDb {
public:
    class Row {
    public:
        std::string Text(int col) const;
        int64_t Int(int col) const;

    private:
        friend class AppDb;
        explicit Row(sqlite3_stmt *stmt) : stmt_(stmt) {}
        sqlite3_stmt *stmt_;
    };

    AppDb() = default;
    ~AppDb();
    AppDb(const AppDb &) = delete;
    AppDb &operator=(const AppDb &) = delete;

    bool Open(const std::string &path, std::string *err);

    bool ForEachRow(const std::string &sql, const std::vector<std::string> &args,
                    const std::function<void(const Row &)> &fn) const;

    std::vector<std::string> Tables(const std::string &like = "%") const;

    std::vector<std::string> Columns(const std::string &table) const;
    int64_t RowCount(const std::string &table) const;

    std::vector<std::vector<std::pair<std::string, std::string>>> Sample(const std::string &table, int limit) const;

    bool BackupTo(const std::string &path, std::string *err) const;

private:
    static bool IsSafeIdentifier(const std::string &name);

    sqlite3 *db_ = nullptr;
};

}
