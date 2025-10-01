#include <gtest/gtest.h>
#include "zp_cpp/math.hpp"

#include <array>
#include <cmath>
#include <vector>

// =========================================================================================================================================
// =========================================================================================================================================
// SampleBezierTrisProducesExpectedVertexCount: Validates triangle strip count for sampled cubic Bézier segments.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathSplinesTest, SampleBezierTrisProducesExpectedVertexCount)
{
    std::array<zp::math::vec2, 4> cp{
        zp::math::vec2{ 0.0f,  0.0f},
        zp::math::vec2{0.25f, 0.75f},
        zp::math::vec2{0.75f, 0.25f},
        zp::math::vec2{ 1.0f,  1.0f}
    };
    const int segments               = 4;

    std::vector<zp::math::vec2> tris = zp::math::splines::sample_bezier_tris({1.0f, 1.0f}, cp, segments, 0.05f);

    const int expected_triangles     = segments * 2;
    EXPECT_EQ(tris.size(), expected_triangles * 3);

    const float half_width = 0.05f * 0.5f;
    EXPECT_LT(std::abs(tris.front().x - cp.front().x), half_width + 1e-4f);
    EXPECT_LT(std::abs(tris.front().y - cp.front().y), half_width + 1e-4f);
    EXPECT_LT(std::abs(tris.back().x - cp.back().x), half_width + 1e-4f);
    EXPECT_LT(std::abs(tris.back().y - cp.back().y), half_width + 1e-4f);
}
