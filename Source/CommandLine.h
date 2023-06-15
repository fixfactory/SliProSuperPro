#pragma once

#include <vector>
#include <string_view>

namespace cmdLine
{
void printHelp();
std::string_view getOption(const std::vector<std::string_view>& args, const std::string_view& optionName);
bool hasOption(const std::vector<std::string_view>& args, const std::string_view& optionName);
bool parseOptions(int argc, char* argv[]);
} // namespace cmdLine
