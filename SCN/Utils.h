#pragma once

#include <string>
#include <windows.h>

namespace utils {
    std::string to_utf8(const std::wstring& wstr);
}
