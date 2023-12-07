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

#include <string>
#include <vector>
#include <algorithm>

#include "CommandLine.h"
#include "Config.h"
#include "Log.h"

namespace cmdLine
{
    void printHelp()
    {
        LOG_INFO("Command line argument usage:");
        LOG_INFO("");
        LOG_INFO("   --help");
        LOG_INFO("      Show this help message.");
        LOG_INFO("");
        LOG_INFO("   --brightness [value]");
        LOG_INFO("      Specify the the brightness level between 0-100.");
        LOG_INFO("      Default: 75");
    }

    std::string_view getOption(const std::vector<std::string_view> &args, const std::string_view &optionName)
    {
        for (auto it = args.begin(), end = args.end(); it != end; ++it)
        {
            if (*it == optionName)
            {
                if (it + 1 != end)
                {
                    return *(it + 1);
                }
            }
        }

        return "";
    }

    bool hasOption(const std::vector<std::string_view> &args, const std::string_view &optionName)
    {
        for (auto it = args.begin(), end = args.end(); it != end; ++it)
        {
            if (*it == optionName)
            {
                return true;
            }
        }

        return false;
    }

    bool parseOptions(int argc, char *argv[])
    {
        // Security check.
        if (argc > 32)
        {
            LOG_ERROR("Too many command line arguments! %i", argc);
            printHelp();
            return false;
        }

        const std::vector<std::string_view> args(argv + 1, argv + argc);

        if (hasOption(args, "--help") || hasOption(args, "-h"))
        {
            printHelp();
            return false;
        }

        if (hasOption(args, "--brightness"))
        {
            std::string option(getOption(args, "--brightness"));
            if (option.empty())
            {
                LOG_ERROR("Invalid brightness.");
                printHelp();
                return false;
            }
            config::brightness = std::clamp<unsigned int>(std::stoi(option), 0, 100);
        }

        if (hasOption(args, "--debugTiming"))
        {
            config::debugTiming = true;
        }

        return true;
    }
} // namespace cmdLine
