#include <gtest/gtest.h>
#include "zp_cpp/core.hpp"
#include "zp_cpp/buff.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// BasicCreation: Validates buffer initializes with zero count.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferTest, BasicCreation)
{
    zp::buff<1024> buffer;
    EXPECT_EQ(buffer.count, 0);
}

// =========================================================================================================================================
// =========================================================================================================================================
// Reset: Validates buffer.reset() resets count to zero.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferTest, Reset)
{
    zp::buff<1024> buffer;
    buffer.count = 500;
    buffer.reset();
    EXPECT_EQ(buffer.count, 0);
}

// =========================================================================================================================================
// =========================================================================================================================================
// BumpAllocate: Validates buffer.bump() allocates sequential memory regions correctly.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferTest, BumpAllocate)
{
    zp::buff<1024> buffer;
    zp::span<std::byte> span1, span2;

    EXPECT_EQ(buffer.bump(100, &span1), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(span1.count, 100);
    EXPECT_EQ(buffer.count, 100);

    EXPECT_EQ(buffer.bump(200, &span2), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(span2.count, 200);
    EXPECT_EQ(buffer.count, 300);

    EXPECT_EQ(span2.p, span1.p + 100);
}

// =========================================================================================================================================
// =========================================================================================================================================
// BumpOutOfBounds: Validates buffer.bump() returns error when exceeding buffer size.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferTest, BumpOutOfBounds)
{
    zp::buff<100> buffer;
    zp::span<std::byte> span;

    EXPECT_EQ(buffer.bump(100, &span), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(buffer.bump(1, &span), zp::Result::ZC_OUT_OF_BOUNDS);
}

// =========================================================================================================================================
// =========================================================================================================================================
// BumpAfterReset: Validates buffer.bump() works correctly after reset.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferTest, BumpAfterReset)
{
    zp::buff<1024> buffer;
    zp::span<std::byte> span1, span2;

    buffer.bump(500, &span1);
    buffer.reset();
    buffer.bump(100, &span2);

    EXPECT_EQ(buffer.count, 100);
    EXPECT_EQ(span2.p, &buffer.bytes[0]);
}

// =========================================================================================================================================
// =========================================================================================================================================
// SpanMethod: Validates buffer.span() returns correct span view of buffer data.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferTest, SpanMethod)
{
    zp::buff<1024> buffer;

    // Test empty buffer - as_span should return full capacity
    auto empty_span = buffer.as_span();
    EXPECT_EQ(empty_span.p, &buffer.bytes[0]);
    EXPECT_EQ(empty_span.count, 1024);

    // Test after bump allocation - as_span should still return full capacity
    zp::span<std::byte> bump_span;
    buffer.bump(200, &bump_span);

    auto full_span = buffer.as_span();
    EXPECT_EQ(full_span.p, &buffer.bytes[0]);
    EXPECT_EQ(full_span.count, 1024);
    EXPECT_EQ(full_span.p, bump_span.p);

    // Test after multiple bumps - as_span should still return full capacity
    buffer.bump(100, &bump_span);
    auto updated_span = buffer.as_span();
    EXPECT_EQ(updated_span.count, 1024);
    EXPECT_EQ(updated_span.p, &buffer.bytes[0]);

    // Test after reset - as_span should still return full capacity
    buffer.reset();
    auto reset_span = buffer.as_span();
    EXPECT_EQ(reset_span.count, 1024);
    EXPECT_EQ(reset_span.p, &buffer.bytes[0]);
}
