#pragma once

#include "core.hpp"
#include "buff.hpp"

#include <filesystem>
#include <unordered_map>
#include <functional>

namespace zp::files
{
    struct dir_watcher
    {
        struct Config
        {
            std::filesystem::path dir;
            std::function<void(const std::filesystem::path&)> on_file_created;
            std::function<void(const std::filesystem::path&)> on_file_modified;
            std::function<void(const std::filesystem::path&)> on_file_destroyed;
        };
        Config config;

        struct State
        {
            std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> known;
        };
        State state;
    };

    Result read_file(const std::filesystem::path& path, span<std::byte> buffer, span<std::byte>* p_out);
    Result write_file(const std::filesystem::path& path, span<const std::byte> data);

    void poll_dir(dir_watcher* p_dir_watcher);
};