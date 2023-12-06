//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2023 Fixfactory
//
// This file is part of SliProSuperPro.
//
// SliProSuperPro is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or any later version.
//
// SliProSuperPro is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with SliProSuperPro. If not, see <http://www.gnu.org/licenses/>.
//

#include <Windows.h>
#include <stdexcept>

#include "StringHelper.h"

namespace string
{
    std::wstring convertToWide(const std::string &string)
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

    std::string convertFromWide(const std::wstring &wideString)
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
