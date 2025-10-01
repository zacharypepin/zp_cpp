#include <gtest/gtest.h>
#include "zp_cpp/hash.hpp"
#include "zp_cpp/core.hpp"
#include "zp_cpp/buff.hpp"
#include <cstring>
#include <unordered_map>

// =========================================================================================================================================
// =========================================================================================================================================
// SpanToHash: Validates span can be hashed and produces correct SHA-256 output length.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(HashIntegrationTest, SpanToHash)
{
    const char* data = "test";
    std::byte bytes[4];
    std::memcpy(bytes, data, 4);

    zp::span<const std::byte> span{bytes, 4};
    auto hash = zp::hash::hash_data(span);

    EXPECT_EQ(zp::hash::to_str(hash).length(), 64);
}

// =========================================================================================================================================
// =========================================================================================================================================
// BufferAndHashWorkflow: Validates buffer allocation combined with span hashing workflow.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(HashIntegrationTest, BufferAndHashWorkflow)
{
    zp::buff<1024> buffer;
    zp::span<std::byte> span1, span2;

    buffer.bump(100, &span1);
    buffer.bump(50, &span2);

    for (size_t i = 0; i < span1.count; ++i)
    {
        span1.p[i] = std::byte{static_cast<uint8_t>(i % 256)};
    }

    zp::span<const std::byte> const_span1{span1.p, span1.count};
    auto hash            = zp::hash::hash_data(const_span1);
    std::string hash_str = zp::hash::to_str(hash);

    EXPECT_EQ(hash_str.length(), 64);
}

// =========================================================================================================================================
// =========================================================================================================================================
// HashInUnorderedMap: Validates hash256 can be used as key in std::unordered_map.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(HashIntegrationTest, HashInUnorderedMap)
{
    std::unordered_map<zp::hash::hash256, std::string> hash_map;

    auto hash1      = zp::hash::hash_data("key1", 4);
    auto hash2      = zp::hash::hash_data("key2", 4);
    auto hash3      = zp::hash::hash_data("key3", 4);

    hash_map[hash1] = "value1";
    hash_map[hash2] = "value2";
    hash_map[hash3] = "value3";

    EXPECT_EQ(hash_map.size(), 3);
    EXPECT_EQ(hash_map[hash1], "value1");
    EXPECT_EQ(hash_map[hash2], "value2");
    EXPECT_EQ(hash_map[hash3], "value3");
}
