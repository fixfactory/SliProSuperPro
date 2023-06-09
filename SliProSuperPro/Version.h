#pragma once

#include <string>
#include <sstream>

namespace version
{
    const std::string kName("Sli-Pro Super Pro");
    constexpr unsigned int kMajor = 0;
    constexpr unsigned int kMinor = 1;
    constexpr unsigned int kRevision = 0;

    std::string getString()
    {
        std::stringstream stream;
        stream << kMajor << "." << kMinor << "." << kRevision;
        return stream.str();
    }
}