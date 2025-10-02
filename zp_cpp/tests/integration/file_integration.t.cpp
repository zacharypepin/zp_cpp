#include <gtest/gtest.h>
#include "zp_cpp/hash.hpp"
#include "zp_cpp/files.hpp"
#include "zp_cpp/buff.hpp"
#include <chrono>
#include <filesystem>
#include <optional>
#include <string_view>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <vector>
#include "../cmn.hpp"

namespace
{
    std::optional<zp::hash::hash256> read_hash(const std::filesystem::path& path)
    {
        zp::buff<2048> buffer;
        zp::span<std::byte> file_data;
        const auto result = zp::files::read_file(path, buffer.as_span(), &file_data);
        if (result != zp::Result::ZC_SUCCESS)
        {
            return std::nullopt;
        }
        return zp::hash::hash_data(zp::span<const std::byte>{file_data.p, file_data.count});
    }

    zp::Result write_text(const std::filesystem::path& path, std::string_view text)
    {
        const zp::span<const std::byte> payload{reinterpret_cast<const std::byte*>(text.data()), text.size()};
        return zp::files::write_file(path, payload);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// DirectoryWatcherMaintainsHashCache: Validates directory polling keeps a hash cache in sync with create/modify/destroy events.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FileHashIntegrationTest, DirectoryWatcherMaintainsHashCache)
{
    const std::filesystem::path watch_dir = zp::test::make_temp_path("zp_cpp_watch");
    ASSERT_TRUE(std::filesystem::create_directory(watch_dir));

    std::vector<std::filesystem::path> created;
    std::vector<std::filesystem::path> modified;
    std::vector<std::filesystem::path> destroyed;
    std::unordered_map<std::filesystem::path, zp::hash::hash256> cache;

    zp::files::dir_watcher watcher;
    watcher.config.dir             = watch_dir;

    watcher.config.on_file_created = [&](const std::filesystem::path& path)
    {
        created.push_back(path);
        if (auto hash = read_hash(path))
        {
            cache[path] = *hash;
        }
    };

    watcher.config.on_file_modified = [&](const std::filesystem::path& path)
    {
        modified.push_back(path);
        if (auto hash = read_hash(path))
        {
            cache[path] = *hash;
        }
    };

    watcher.config.on_file_destroyed = [&](const std::filesystem::path& path)
    {
        destroyed.push_back(path);
        cache.erase(path);
    };

    zp::files::poll_dir(&watcher);
    EXPECT_TRUE(created.empty());
    EXPECT_TRUE(modified.empty());
    EXPECT_TRUE(destroyed.empty());
    EXPECT_TRUE(cache.empty());

    const std::filesystem::path tracked_file = watch_dir / "tracked.txt";
    ASSERT_EQ(write_text(tracked_file, "initial"), zp::Result::ZC_SUCCESS);

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(created.size(), 1u);
    EXPECT_EQ(created.front(), tracked_file);
    ASSERT_TRUE(cache.contains(tracked_file));
    const auto initial_hash = cache.at(tracked_file);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(write_text(tracked_file, "initial with more data"), zp::Result::ZC_SUCCESS);

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(modified.size(), 1u);
    EXPECT_EQ(modified.front(), tracked_file);
    ASSERT_TRUE(cache.contains(tracked_file));
    EXPECT_TRUE(cache.at(tracked_file) != initial_hash);

    ASSERT_TRUE(std::filesystem::remove(tracked_file));

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(destroyed.size(), 1u);
    EXPECT_EQ(destroyed.front(), tracked_file);
    EXPECT_FALSE(cache.contains(tracked_file));

    std::error_code cleanup_ec;
    std::filesystem::remove_all(watch_dir, cleanup_ec);
}
