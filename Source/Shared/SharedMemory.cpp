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
#include "StringHelper.h"

#include "SharedMemory.h"

SharedMemory::SharedMemory(const std::string &name, unsigned int size) 
    : m_mapSize(size)
{
    m_mapName = string::convertToWide(name.c_str());

    m_mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, // Use paging file
                                  nullptr,              // Default security
                                  PAGE_READWRITE,       // Read/write access
                                  0,                    // Maximum object size (high-order DWORD)
                                  size,                 // Maximum object size (low-order DWORD)
                                  m_mapName.c_str());   // Name of mapping object

    if (m_mapFile == nullptr)
    {
        if (GetLastError() == static_cast<DWORD>(183)) 
        {
            // File mapping already exists; try opening.
            m_mapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, m_mapName.c_str());
            if (m_mapFile == nullptr)
            {
                LOG_ERROR("Could not open existing file mapping.");
                return;
            }
        }
        else
        {
            LOG_ERROR("Could not create file mapping object");
            return;
        }
    }
    
    m_buffer = static_cast<void *>(MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, size));
    if (m_buffer == nullptr)
    {
        LOG_ERROR("Could not reserve buffer for shared memory");
        CloseHandle(m_mapFile);
        return;
    }
    
    memset(m_buffer, 0, size);
    m_isHooked = true;
    LOG_INFO("Memory file mapping hooked.");
}

SharedMemory::~SharedMemory()
{
    if (m_isHooked)
    {
        if (m_buffer != nullptr)
        {
            UnmapViewOfFile(m_buffer);
        }

        if (m_mapFile != nullptr)
        {
            CloseHandle(m_mapFile);
        }
    }

    m_isHooked = false;
}
