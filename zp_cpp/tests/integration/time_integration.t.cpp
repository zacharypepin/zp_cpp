#include <gtest/gtest.h>
#include "zp_cpp/core.hpp"
#include "zp_cpp/time.hpp"
#include <thread>
#include <chrono>

// =========================================================================================================================================
// =========================================================================================================================================
// InterpolationProgressesAndWraps: Validates calc_interp() tracks progress and wraps after exceeding duration.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(TimeIntegrationTest, InterpolationProgressesAndWraps)
{
    const zp::ens start    = zp::now();
    const zp::ens duration = 50'000'000ULL; // 50 ms

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    const zp::ens mid        = zp::now();
    const float first_interp = zp::calc_interp(start, mid, duration);
    EXPECT_GT(first_interp, 0.0f);
    EXPECT_LT(first_interp, 1.0f);

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    const zp::ens later        = zp::now();
    const float wrapped_interp = zp::calc_interp(start, later, duration);

    EXPECT_GT(later - start, duration);
    EXPECT_GE(wrapped_interp, 0.0f);
    EXPECT_LT(wrapped_interp, 1.0f);
    EXPECT_LT(wrapped_interp, first_interp);
}
