#include <gtest/gtest.h>
#include "zp_cpp/core.hpp"
#include "zp_cpp/files.hpp"
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <thread>
#include <vector>
#include "../cmn.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// WriteAndReadFile: Validates write_file() and read_file() work correctly for binary data.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FilesTest, WriteAndReadFile)
{
    const std::filesystem::path test_file = zp::test::make_temp_path("zp_cpp_test_file", ".txt");
    const char* write_data                = "Hello, World!";
    const size_t write_size               = std::strlen(write_data);

    // Create span for write data
    zp::span<const std::byte> write_span{reinterpret_cast<const std::byte*>(write_data), write_size};
    ASSERT_EQ(zp::files::write_file(test_file, write_span), zp::Result::ZC_SUCCESS);

    // Create buffer and span for read data
    zp::buff<1024> read_buffer;
    zp::span<std::byte> read_span = read_buffer.as_span();
    zp::span<std::byte> actual_data;
    ASSERT_EQ(zp::files::read_file(test_file, read_span, &actual_data), zp::Result::ZC_SUCCESS);

    EXPECT_EQ(actual_data.count, write_size);
    EXPECT_EQ(std::memcmp(actual_data.p, write_data, write_size), 0);

    std::error_code ec;
    std::filesystem::remove(test_file, ec);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ReadNonExistentFile: Validates read_file() returns ZC_FILE_NOT_FOUND for non-existent files.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FilesTest, ReadNonExistentFile)
{
    const std::filesystem::path test_file = zp::test::make_temp_path("zp_cpp_missing_file", ".txt");

    zp::buff<1024> buffer;
    zp::span<std::byte> read_span = buffer.as_span();
    zp::span<std::byte> actual_data;

    EXPECT_EQ(zp::files::read_file(test_file, read_span, &actual_data), zp::Result::ZC_FILE_NOT_FOUND);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ReadFileBufferTooSmall: Validates read_file() returns ZC_OUT_OF_BOUNDS when buffer cannot hold file contents.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FilesTest, ReadFileBufferTooSmall)
{
    const std::filesystem::path test_file = zp::test::make_temp_path("zp_cpp_small_buffer", ".bin");

    {
        std::ofstream ofs(test_file, std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs << "abcdef";
    }

    zp::buff<4> buffer;
    zp::span<std::byte> read_span = buffer.as_span();
    zp::span<std::byte> actual_data;

    EXPECT_EQ(zp::files::read_file(test_file, read_span, &actual_data), zp::Result::ZC_OUT_OF_BOUNDS);

    std::error_code ec;
    std::filesystem::remove(test_file, ec);
}

// =========================================================================================================================================
// =========================================================================================================================================
// WriteToInvalidPath: Validates write_file() returns ZC_FILE_ACCESS_ERROR for invalid paths.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FilesTest, WriteToInvalidPath)
{
    const std::filesystem::path invalid_path = zp::test::make_temp_path("zp_cpp_invalid_dir") / "file.txt";
    const char* data                         = "test";

    zp::span<const std::byte> data_span{reinterpret_cast<const std::byte*>(data), 4};
    EXPECT_EQ(zp::files::write_file(invalid_path, data_span), zp::Result::ZC_FILE_ACCESS_ERROR);
}

// =========================================================================================================================================
// =========================================================================================================================================
// PollDirLifecycle: Validates poll_dir() reports file creation, modification, and destruction exactly once per event.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FilesTest, PollDirLifecycle)
{
    const std::filesystem::path watch_dir = zp::test::make_temp_path("zp_cpp_dir_watcher");
    std::error_code ec;
    std::filesystem::create_directories(watch_dir, ec);
    ASSERT_FALSE(ec);

    zp::files::dir_watcher watcher;
    watcher.config.dir = watch_dir;

    std::vector<std::filesystem::path> created;
    std::vector<std::filesystem::path> modified;
    std::vector<std::filesystem::path> destroyed;

    watcher.config.on_file_created        = [&](const std::filesystem::path& path) { created.push_back(path); };
    watcher.config.on_file_modified       = [&](const std::filesystem::path& path) { modified.push_back(path); };
    watcher.config.on_file_destroyed      = [&](const std::filesystem::path& path) { destroyed.push_back(path); };

    const std::filesystem::path file_path = watch_dir / "watched.txt";
    {
        std::ofstream ofs(file_path, std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs << "initial";
    }

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(created.size(), 1u);
    EXPECT_EQ(created.front(), file_path);
    EXPECT_TRUE(modified.empty());
    EXPECT_TRUE(destroyed.empty());

    // A second poll without changes should not emit additional events.
    zp::files::poll_dir(&watcher);
    EXPECT_EQ(created.size(), 1u);
    EXPECT_TRUE(modified.empty());
    EXPECT_TRUE(destroyed.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    {
        std::ofstream ofs(file_path, std::ios::app | std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs << "update";
    }

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(modified.size(), 1u);
    EXPECT_EQ(modified.front(), file_path);

    std::filesystem::remove(file_path, ec);
    ASSERT_FALSE(ec);

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(destroyed.size(), 1u);
    EXPECT_EQ(destroyed.front(), file_path);

    ec.clear();
    std::filesystem::remove_all(watch_dir, ec);
    EXPECT_FALSE(ec);
}

// =========================================================================================================================================
// =========================================================================================================================================
// HasChangedDetectsFileUpdates: Validates has_changed() tracks initial state and subsequent modifications precisely.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FilesTest, HasChangedDetectsFileUpdates)
{
    const std::filesystem::path file_path = zp::test::make_temp_path("zp_cpp_file_watcher", ".txt");

    {
        std::ofstream ofs(file_path, std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs << "initial";
    }

    zp::files::file_watcher_t watcher;
    watcher.config.path = file_path;

    // First poll should detect that the watcher has no prior timestamp recorded.
    EXPECT_TRUE(zp::files::has_changed(&watcher));

    // Without any modifications the subsequent poll must not report a change.
    EXPECT_FALSE(zp::files::has_changed(&watcher));

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    {
        std::ofstream ofs(file_path, std::ios::app | std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs << "update";
    }

    // File modification should be detected, with the watcher state updated accordingly.
    EXPECT_TRUE(zp::files::has_changed(&watcher));
    EXPECT_FALSE(zp::files::has_changed(&watcher));

    std::error_code ec;
    std::filesystem::remove(file_path, ec);
}
