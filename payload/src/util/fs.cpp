#include "util/fs.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>

namespace playtrack::fs {
std::optional<Stat> StatPath(const std::string &path) {
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) return std::nullopt;
    Stat out;
    out.is_dir = S_ISDIR(st.st_mode);
    out.size = st.st_size;
    out.mtime = st.st_mtime;
    return out;
}

bool IsNonEmptyFile(const std::string &path) {
    auto st = StatPath(path);
    return st && !st->is_dir && st->size > 0;
}

std::vector<Entry> ListDir(const std::string &path) {
    std::vector<Entry> entries;
    DIR *dir = opendir(path.c_str());
    if (!dir) return entries;
    while (dirent *ent = readdir(dir)) {
        std::string name = ent->d_name;
        if (name == "." || name == "..") continue;
        Entry e;
        e.name = name;
        if (auto st = StatPath(path + "/" + name)) {
            e.is_dir = st->is_dir;
            e.size = st->size;
        }
        entries.push_back(std::move(e));
    }
    closedir(dir);
    std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) { return a.name < b.name; });
    return entries;
}

bool ReadRange(const std::string &path, uint64_t offset, int64_t length, std::string *out, int64_t max_bytes) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return false;
    struct stat st {};
    int64_t size = fstat(fd, &st) == 0 ? st.st_size : -1;
    if (length < 0) length = size - static_cast<int64_t>(offset);
    if (size < 0 || length < 0 || length > max_bytes || static_cast<int64_t>(offset) + length > size) {
        close(fd);
        return false;
    }

    out->resize(static_cast<size_t>(length));
    size_t done = 0;
    while (done < out->size()) {
        ssize_t n = pread(fd, out->data() + done, out->size() - done, static_cast<off_t>(offset + done));
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) break;
        done += static_cast<size_t>(n);
    }
    close(fd);
    return done == out->size();
}

bool ReadFile(const std::string &path, std::string *out, int64_t max_bytes) {
    return ReadRange(path, 0, -1, out, max_bytes);
}

bool WriteFileAtomic(const std::string &path, std::string_view data) {
    std::string tmp = path + ".tmp";
    int fd = open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return false;
    size_t done = 0;
    while (done < data.size()) {
        ssize_t n = write(fd, data.data() + done, data.size() - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) {
            close(fd);
            unlink(tmp.c_str());
            return false;
        }
        done += static_cast<size_t>(n);
    }
    fsync(fd);
    close(fd);
    return rename(tmp.c_str(), path.c_str()) == 0;
}

bool EnsureDir(const std::string &path) {
    if (path.empty()) return false;
    for (size_t pos = 1; pos <= path.size(); ++pos) {
        if (pos != path.size() && path[pos] != '/') continue;
        std::string part = path.substr(0, pos);
        if (mkdir(part.c_str(), 0755) != 0 && errno != EEXIST) return false;
    }
    auto st = StatPath(path);
    return st && st->is_dir;
}

}
