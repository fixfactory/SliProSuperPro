//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2024 Fixfactory
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

#pragma once

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

class SharedMemory
{
public:
    SharedMemory(const std::string &name, unsigned int size, bool openExistingOnly = false);
    ~SharedMemory();

    bool isHooked() { return m_buffer != nullptr; }
    void *getBuffer() { return m_buffer; }

protected:
    HANDLE m_mapFile{ nullptr };
    void *m_buffer{ nullptr };
};