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

#include "Log.h"
#include "SharedMemory.h"

SharedMemory::SharedMemory() 
{
}

SharedMemory::~SharedMemory()
{
    close();
}

bool SharedMemory::open(const std::string &name, unsigned int size, bool openExistingOnly)
{
    if (isOpened())
    {
        close();
    }

    if (!openExistingOnly)
    {
        LOG_DEBUG("Creating memory file %s size=%i", name.c_str(), size);
        m_mapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, // Use paging file
                                       nullptr,              // Default security
                                       PAGE_READWRITE,       // Read/write access
                                       0,                    // Maximum object size (high-order DWORD)
                                       size,                 // Maximum object size (low-order DWORD)
                                       name.c_str());        // Name of mapping object
    }

    if (m_mapFile == nullptr)
    {
        LOG_DEBUG("Try opening memory file %s size=%i", name.c_str(), size);
        m_mapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, name.c_str());
    }

    if (m_mapFile == nullptr)
    {
        LOG_DEBUG("Could not open file mapping.");
        return false;
    }

    m_buffer = static_cast<void *>(MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, size));
    if (m_buffer == nullptr)
    {
        LOG_DEBUG("Could not reserve buffer for shared memory");
        CloseHandle(m_mapFile);
        return false;
    }

    LOG_DEBUG("Memory file opened successfully.");
    return true;
}

void SharedMemory::close()
{
    if (m_buffer != nullptr)
    {
        UnmapViewOfFile(m_buffer);
        m_buffer = nullptr;
    }

    if (m_mapFile != nullptr)
    {
        CloseHandle(m_mapFile);
        m_mapFile = nullptr;
    }
}
