#include "zp_cpp/gpu.hpp"

#include <gtest/gtest.h>

TEST(GpuUtilAlignedSizeTest, HandlesUint32Basics)
{
    EXPECT_EQ(zp::gpu::util::aligned_size(uint32_t(0), 8), 0u);
    EXPECT_EQ(zp::gpu::util::aligned_size(uint32_t(1), 8), 8u);
    EXPECT_EQ(zp::gpu::util::aligned_size(uint32_t(32), 16), 32u);
    EXPECT_EQ(zp::gpu::util::aligned_size(uint32_t(33), 16), 48u);
}

TEST(GpuUtilAlignedSizeTest, HandlesSizeTBasics)
{
    EXPECT_EQ(zp::gpu::util::aligned_size(size_t(1), size_t(64)), size_t(64));
    EXPECT_EQ(zp::gpu::util::aligned_size(size_t(1025), size_t(512)), size_t(1536));
}

TEST(GpuUtilAlignedSizeTest, HandlesVkDeviceSizeBasics)
{
    EXPECT_EQ(zp::gpu::util::aligned_vk_size(VkDeviceSize(1000), VkDeviceSize(256)), VkDeviceSize(1024));
}
