#pragma once

#include <string>

namespace string
{
std::wstring convertToWide(const std::string& string);
std::string convertFromWide(const std::wstring& wideString);
} // namespace string
