#include <gtest/gtest.h>
#include "zp_cpp/hash.hpp"
#include "zp_cpp/files.hpp"
#include "zp_cpp/buff.hpp"
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <unordered_map>
#include "../cmn.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// HashCacheUpdatesOnFileChange: Validates hash caching over file contents reacts to real file modifications.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(HashIntegrationTest, HashCacheUpdatesOnFileChange)
{
    const std::filesystem::path file_path = zp::test::make_temp_path("zp_cpp_hash_cache", ".txt");

    const auto write_contents             = [&](std::string_view contents)
    {
        const zp::span<const std::byte> data{reinterpret_cast<const std::byte*>(contents.data()), contents.size()};
        return zp::files::write_file(file_path, data);
    };

    ASSERT_EQ(write_contents("alpha"), zp::Result::ZC_SUCCESS);

    std::unordered_map<std::filesystem::path, zp::hash::hash256> cache;
    zp::buff<2048> scratch;

    const auto load_hash = [&]()
    {
        zp::span<std::byte> file_data;
        const auto result = zp::files::read_file(file_path, scratch.as_span(), &file_data);
        EXPECT_EQ(result, zp::Result::ZC_SUCCESS);
        if (result != zp::Result::ZC_SUCCESS)
        {
            return zp::hash::hash256{};
        }
        return zp::hash::hash_data(zp::span<const std::byte>{file_data.p, file_data.count});
    };

    const auto first_hash = load_hash();
    cache[file_path]      = first_hash;

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(write_contents("alpha beta gamma"), zp::Result::ZC_SUCCESS);

    const auto second_hash = load_hash();
    EXPECT_TRUE(first_hash != second_hash);

    cache[file_path] = second_hash;
    EXPECT_EQ(cache.size(), 1u);
    EXPECT_TRUE(cache.at(file_path) == second_hash);

    zp::buff<2048> verify_buffer;
    zp::span<std::byte> verify_span;
    ASSERT_EQ(zp::files::read_file(file_path, verify_buffer.as_span(), &verify_span), zp::Result::ZC_SUCCESS);
    const auto verify_hash = zp::hash::hash_data(zp::span<const std::byte>{verify_span.p, verify_span.count});
    EXPECT_TRUE(verify_hash == second_hash);

    std::error_code ec;
    std::filesystem::remove(file_path, ec);
}
