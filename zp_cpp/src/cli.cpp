#include "zp_cpp/cli.hpp"

#include <string_view>

// =========================================================================================================================================
// =========================================================================================================================================
// parse_cli: Parses command-line arguments in --key=value format. Returns map of key-value pairs, skipping invalid entries.
// =========================================================================================================================================
// =========================================================================================================================================
std::unordered_map<std::string, std::string> zp::cli::parse_cli(int argc, char** argv)
{
    std::unordered_map<std::string, std::string> out;

    for (int i = 1; i < argc; ++i)
    {
        const char* raw = argv[i];
        if (!raw)
        {
            continue;
        }

        std::string_view arg{raw};

        if (arg.size() < 2)
        {
            continue;
        }

        if (!(arg[0] == '-' && arg[1] == '-'))
        {
            continue;
        }

        arg.remove_prefix(2);
        const size_t eq_pos = arg.find('=');
        if (eq_pos == std::string_view::npos)
        {
            continue;
        }

        std::string_view key = arg.substr(0, eq_pos);
        std::string_view val = arg.substr(eq_pos + 1);

        if (key.empty())
        {
            continue;
        }

        out[std::string(key)] = std::string(val);
    }

    return out;
}
