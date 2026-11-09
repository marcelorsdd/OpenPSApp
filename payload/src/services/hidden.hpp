#pragma once

#include <string>

namespace playtrack::hidden {
void Load(const std::string &data_dir);
bool Contains(const std::string &title_id);

bool Set(const std::string &title_id, bool hide);

}
