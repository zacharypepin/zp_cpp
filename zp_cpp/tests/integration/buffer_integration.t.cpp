#include <gtest/gtest.h>
#include "zp_cpp/core.hpp"
#include "zp_cpp/buff.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// ComplexDataManipulation: Validates complex workflow using buffer, span copy, and receive operations.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferSpanIntegrationTest, ComplexDataManipulation)
{
    zp::buff<1024> buffer;
    zp::span<std::byte> span1, span2, span3;

    buffer.bump(100, &span1);
    buffer.bump(200, &span2);
    buffer.bump(50, &span3);

    for (size_t i = 0; i < span1.count; ++i)
    {
        span1.p[i] = std::byte{static_cast<uint8_t>(i)};
    }

    uint8_t temp[100];
    zp::span<std::byte> temp_span{reinterpret_cast<std::byte*>(temp), 100};
    EXPECT_EQ(span1.cpy(0, 100, temp), zp::Result::ZC_SUCCESS);

    EXPECT_EQ(span2.rcv(50, 100, temp), zp::Result::ZC_SUCCESS);

    for (size_t i = 0; i < 100; ++i)
    {
        EXPECT_EQ(span2.p[50 + i], span1.p[i]);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// SpanSlicing: Validates creating nested subspans through multiple split() calls.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(BufferSpanIntegrationTest, SpanSlicing)
{
    int data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    zp::span<int> full_span{data, 10};

    zp::span<int> first, second, third, fourth;

    // Split at position 3: [0,1,2] and [3,4,5,6,7,8,9]
    EXPECT_EQ(full_span.split(3, &first, &second), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(first.p, &data[0]);
    EXPECT_EQ(first.count, 3);
    EXPECT_EQ(second.p, &data[3]);
    EXPECT_EQ(second.count, 7);

    // Split second part at position 2: [3,4] and [5,6,7,8,9]
    EXPECT_EQ(second.split(2, &third, &fourth), zp::Result::ZC_SUCCESS);
    EXPECT_EQ(third.p, &data[3]);
    EXPECT_EQ(third.count, 2);
    EXPECT_EQ(fourth.p, &data[5]);
    EXPECT_EQ(fourth.count, 5);
    EXPECT_EQ(fourth.p[0], 5);
}
