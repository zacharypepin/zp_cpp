#include "zp_cpp/files.hpp"

#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <vector>

// =========================================================================================================================================
// =========================================================================================================================================
// read_file: Modern span-based version that reads file contents into a span buffer with proper error handling.
// =========================================================================================================================================
// =========================================================================================================================================
zp::Result zp::files::read_file(const std::filesystem::path& path, zp::span<std::byte> buffer, zp::span<std::byte>* p_out)
{
    if (!std::filesystem::exists(path))
    {
        return zp::Result::ZC_FILE_NOT_FOUND;
    }

    const size_t file_size = std::filesystem::file_size(path);
    if (file_size > buffer.count)
    {
        return zp::Result::ZC_OUT_OF_BOUNDS;
    }

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
    {
        return zp::Result::ZC_FILE_ACCESS_ERROR;
    }

    zp::span<std::byte> file_span;
    zp::span<std::byte> unused_span;
    zp::Result split_result = buffer.split(file_size, &file_span, &unused_span);
    if (split_result != zp::Result::ZC_SUCCESS)
    {
        return split_result;
    }

    ifs.read(reinterpret_cast<char*>(file_span.p), file_size);
    if (!ifs.good())
    {
        return zp::Result::ZC_FILE_READ_ERROR;
    }

    *p_out = file_span;
    return zp::Result::ZC_SUCCESS;
}

// =========================================================================================================================================
// =========================================================================================================================================
// write_file: Modern span-based version that writes span data to file with proper error handling.
// =========================================================================================================================================
// =========================================================================================================================================
zp::Result zp::files::write_file(const std::filesystem::path& path, zp::span<const std::byte> data)
{
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
    {
        return zp::Result::ZC_FILE_ACCESS_ERROR;
    }

    ofs.write(reinterpret_cast<const char*>(data.p), data.count);
    ofs.close();

    if (!ofs.good())
    {
        return zp::Result::ZC_FILE_WRITE_ERROR;
    }

    return zp::Result::ZC_SUCCESS;
}

// =========================================================================================================================================
// =========================================================================================================================================
// poll_dir: Polls directory for file changes, invoking callbacks for created, modified, or destroyed files.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::files::poll_dir(dir_watcher* p_dir_watcher)
{
    // ====================================================================================================
    // ====================================================================================================
    // Detect new and modified files by comparing last write times against known state.
    // ====================================================================================================
    // ====================================================================================================
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(p_dir_watcher->config.dir))
        {
            if (!std::filesystem::is_regular_file(entry))
            {
                continue;
            }

            const std::filesystem::path& path                = entry.path();
            const std::filesystem::file_time_type& last_time = entry.last_write_time();

            if (!p_dir_watcher->state.known.contains(path))
            {
                p_dir_watcher->state.known.insert({path, last_time});
                p_dir_watcher->config.on_file_created(path);
            }
            else if (p_dir_watcher->state.known.at(path) != last_time)
            {
                p_dir_watcher->state.known[path] = last_time;
                p_dir_watcher->config.on_file_modified(path);
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // Detect destroyed files by checking if known paths still exist on filesystem.
    // ====================================================================================================
    // ====================================================================================================
    {
        std::vector<std::filesystem::path> destroyed;
        for (auto&& [path, last_write] : p_dir_watcher->state.known)
        {
            if (!std::filesystem::exists(path))
            {
                destroyed.push_back(path);
            }
        }

        for (auto&& path : destroyed)
        {
            p_dir_watcher->state.known.erase(path);
            p_dir_watcher->config.on_file_destroyed(path);
        }
    }
}
