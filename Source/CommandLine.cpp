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
    LOG_INFO("   --port [port number]");
    LOG_INFO("      Specify the UDP port number to listen to telemetry.");
    LOG_INFO("      Default: 6776");
    LOG_INFO("");
    LOG_INFO("   --brightness [value]");
    LOG_INFO("      Specify the the brightness level between 0-100.");
    LOG_INFO("      Default: 75");
}

std::string_view getOption(const std::vector<std::string_view>& args, const std::string_view& optionName)
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

bool hasOption(const std::vector<std::string_view>& args, const std::string_view& optionName)
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

bool parseOptions(int argc, char* argv[])
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

    if (hasOption(args, "--port"))
    {
        std::string option(getOption(args, "--port"));
        int port = std::stoi(option);
        if (option.empty() || port > 65535)
        {
            LOG_ERROR("Invalid port number. %i", port);
            printHelp();
            return false;
        }
        config::udpPort = port;
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
