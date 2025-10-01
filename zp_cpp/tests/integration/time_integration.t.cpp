#include <gtest/gtest.h>
#include "zp_cpp/core.hpp"
#include "zp_cpp/time.hpp"
#include <thread>
#include <chrono>

// =========================================================================================================================================
// =========================================================================================================================================
// InterpolationWithRealTime: Validates calc_interp() with real system time produces valid interpolation values.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(TimeIntegrationTest, InterpolationWithRealTime)
{
    auto start    = zp::now();
    auto duration = 100000000ULL;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto current = zp::now();
    float interp = zp::calc_interp(start, current, duration);

    EXPECT_GE(interp, 0.0f);
    EXPECT_LE(interp, 1.0f);
}
