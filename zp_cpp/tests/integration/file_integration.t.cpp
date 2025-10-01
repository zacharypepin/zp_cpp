#include <gtest/gtest.h>
#include "zp_cpp/hash.hpp"
#include "zp_cpp/core.hpp"
#include "zp_cpp/files.hpp"
#include <cstring>
#include <filesystem>
#include <chrono>
#include <string>
#include <system_error>
#include <thread>
#include <vector>
#include "../cmn.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// HashFileContents: Validates file contents can be read and hashed correctly.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FileHashIntegrationTest, HashFileContents)
{
    const std::filesystem::path test_file = zp::test::make_temp_path("zp_cpp_hash_test", ".txt");
    const char* content                   = "This is test content for hashing";
    const size_t content_size             = std::strlen(content);

    zp::span<const std::byte> content_span{reinterpret_cast<const std::byte*>(content), content_size};
    ASSERT_EQ(zp::files::write_file(test_file, content_span), zp::Result::ZC_SUCCESS);

    zp::buff<1024> buffer;
    zp::span<std::byte> read_span = buffer.as_span();
    zp::span<std::byte> actual_data;
    ASSERT_EQ(zp::files::read_file(test_file, read_span, &actual_data), zp::Result::ZC_SUCCESS);

    zp::span<const std::byte> const_actual_data{actual_data.p, actual_data.count};
    auto hash          = zp::hash::hash_data(const_actual_data);

    auto expected_hash = zp::hash::hash_data(content_span);

    EXPECT_TRUE(hash == expected_hash);

    std::error_code ec;
    std::filesystem::remove(test_file, ec);
}

// =========================================================================================================================================
// =========================================================================================================================================
// HashMultipleFiles: Validates files with same content produce identical hashes.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FileHashIntegrationTest, HashMultipleFiles)
{
    const std::filesystem::path file1 = zp::test::make_temp_path("zp_cpp_file1", ".txt");
    const std::filesystem::path file2 = zp::test::make_temp_path("zp_cpp_file2", ".txt");
    const std::filesystem::path file3 = zp::test::make_temp_path("zp_cpp_file3", ".txt");

    zp::span<const std::byte> content1_span{reinterpret_cast<const std::byte*>("content1"), 8};
    zp::span<const std::byte> content2_span{reinterpret_cast<const std::byte*>("content2"), 8};
    zp::span<const std::byte> content3_span{reinterpret_cast<const std::byte*>("content1"), 8};

    ASSERT_EQ(zp::files::write_file(file1, content1_span), zp::Result::ZC_SUCCESS);
    ASSERT_EQ(zp::files::write_file(file2, content2_span), zp::Result::ZC_SUCCESS);
    ASSERT_EQ(zp::files::write_file(file3, content3_span), zp::Result::ZC_SUCCESS);

    zp::buff<1024> buffer;
    zp::span<std::byte> read_span = buffer.as_span();
    zp::span<std::byte> actual_data;

    ASSERT_EQ(zp::files::read_file(file1, read_span, &actual_data), zp::Result::ZC_SUCCESS);
    zp::span<const std::byte> const_data1{actual_data.p, actual_data.count};
    auto hash1 = zp::hash::hash_data(const_data1);

    ASSERT_EQ(zp::files::read_file(file2, read_span, &actual_data), zp::Result::ZC_SUCCESS);
    zp::span<const std::byte> const_data2{actual_data.p, actual_data.count};
    auto hash2 = zp::hash::hash_data(const_data2);

    ASSERT_EQ(zp::files::read_file(file3, read_span, &actual_data), zp::Result::ZC_SUCCESS);
    zp::span<const std::byte> const_data3{actual_data.p, actual_data.count};
    auto hash3 = zp::hash::hash_data(const_data3);

    EXPECT_TRUE(hash1 == hash3);
    EXPECT_FALSE(hash1 == hash2);

    std::error_code ec1;
    std::filesystem::remove(file1, ec1);
    std::error_code ec2;
    std::filesystem::remove(file2, ec2);
    std::error_code ec3;
    std::filesystem::remove(file3, ec3);
}

// =========================================================================================================================================
// =========================================================================================================================================
// DirectoryPollingLifecycle: Validates dir_watcher detects create, modify, and destroy events.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(FileHashIntegrationTest, DirectoryPollingLifecycle)
{
    const std::filesystem::path watch_dir = zp::test::make_temp_path("zp_cpp_watch");
    ASSERT_TRUE(std::filesystem::create_directory(watch_dir));

    std::vector<std::filesystem::path> created;
    std::vector<std::filesystem::path> modified;
    std::vector<std::filesystem::path> destroyed;

    zp::files::dir_watcher watcher;
    watcher.config.dir               = watch_dir;
    watcher.config.on_file_created   = [&](const std::filesystem::path& path) { created.push_back(path); };
    watcher.config.on_file_modified  = [&](const std::filesystem::path& path) { modified.push_back(path); };
    watcher.config.on_file_destroyed = [&](const std::filesystem::path& path) { destroyed.push_back(path); };

    // Initial poll should observe nothing.
    zp::files::poll_dir(&watcher);
    EXPECT_TRUE(created.empty());
    EXPECT_TRUE(modified.empty());
    EXPECT_TRUE(destroyed.empty());

    const std::filesystem::path file_path = watch_dir / "tracked.txt";
    const char* contents                  = "initial";
    zp::span<const std::byte> initial_span{reinterpret_cast<const std::byte*>(contents), std::strlen(contents)};
    ASSERT_EQ(zp::files::write_file(file_path, initial_span), zp::Result::ZC_SUCCESS);

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(created.size(), 1);
    EXPECT_EQ(created.front(), file_path);
    EXPECT_TRUE(modified.empty());
    EXPECT_TRUE(destroyed.empty());

    // Modify the file and ensure the watcher reports a modification.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const char* updated_contents = "updated";
    zp::span<const std::byte> updated_span{reinterpret_cast<const std::byte*>(updated_contents), std::strlen(updated_contents)};
    ASSERT_EQ(zp::files::write_file(file_path, updated_span), zp::Result::ZC_SUCCESS);

    zp::files::poll_dir(&watcher);
    ASSERT_EQ(modified.size(), 1);
    EXPECT_EQ(modified.front(), file_path);
    EXPECT_TRUE(destroyed.empty());

    ASSERT_TRUE(std::filesystem::remove(file_path));
    zp::files::poll_dir(&watcher);
    ASSERT_EQ(destroyed.size(), 1);
    EXPECT_EQ(destroyed.front(), file_path);

    std::error_code cleanup_ec;
    std::filesystem::remove_all(watch_dir, cleanup_ec);
}
