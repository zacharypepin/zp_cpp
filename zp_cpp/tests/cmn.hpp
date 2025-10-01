#pragma once

#include <chrono>
#include <filesystem>
#include <string>

namespace zp::test
{
    inline std::filesystem::path make_temp_path(const std::string& prefix, const std::string& suffix = "")
    {
        static std::size_t counter  = 0;

        const auto unique           = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        std::string filename        = prefix;
        filename                   += "_";
        filename                   += std::to_string(unique);
        filename                   += "_";
        filename                   += std::to_string(counter++);

        if (!suffix.empty())
        {
            filename += suffix;
        }

        return std::filesystem::temp_directory_path() / filename;
    }
}
