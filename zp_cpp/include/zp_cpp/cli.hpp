#pragma once

#include <unordered_map>
#include <string>

namespace zp::cli
{
    std::unordered_map<std::string, std::string> parse_cli(int argc, char** argv);
};
