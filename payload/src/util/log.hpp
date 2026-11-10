#pragma once

#include <string>
#include <vector>

namespace openpsapp::log {
void Init(const std::string &file_path);
void Info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void Error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
std::vector<std::string> Tail(size_t lines);

int RawFd();

}
