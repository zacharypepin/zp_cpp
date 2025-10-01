#include <gtest/gtest.h>
#include "zp_cpp/core.hpp"
#include "zp_cpp/files.hpp"
#include <cstring>
#include <filesystem>
#include <system_error>
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
