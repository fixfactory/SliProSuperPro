#include "String.h"

#include <stdexcept>
#include <Windows.h>

namespace string
{
std::wstring convertToWide(const std::string& string)
{
    if (string.empty())
    {
        return L"";
    }

    const auto size = MultiByteToWideChar(CP_UTF8, 0, &string.at(0), (int)string.size(), nullptr, 0);
    if (size <= 0)
    {
        throw std::runtime_error("MultiByteToWideChar() failed with size: " + std::to_string(size));
    }

    std::wstring ret(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &string.at(0), (int)string.size(), &ret.at(0), size);
    return ret;
}

std::string convertFromWide(const std::wstring& wideString)
{
    if (wideString.empty())
    {
        return "";
    }

    const auto size =
        WideCharToMultiByte(CP_UTF8, 0, &wideString.at(0), (int)wideString.size(), nullptr, 0, nullptr, nullptr);
    if (size <= 0)
    {
        throw std::runtime_error("WideCharToMultiByte() failed with size: " + std::to_string(size));
    }

    std::string ret(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wideString.at(0), (int)wideString.size(), &ret.at(0), size, nullptr, nullptr);
    return ret;
}
} // namespace string
