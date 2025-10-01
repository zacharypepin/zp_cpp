#include <gtest/gtest.h>
#include "zp_cpp/math.hpp"

#include <array>
#include <cmath>
#include <string>

// =========================================================================================================================================
// =========================================================================================================================================
// ToStringFormatsAllTypes: Validates to_string() variants emit canonical tuple layouts.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMiscTest, ToStringFormatsAllTypes)
{
    EXPECT_EQ(zp::math::to_string(zp::math::vec2{1.0f, 2.0f}), "(1, 2)");
    EXPECT_EQ(zp::math::to_string(zp::math::vec3{1.0f, 2.0f, 3.0f}), "(1, 2, 3)");
    EXPECT_EQ(zp::math::to_string(zp::math::vec4{1.0f, 2.0f, 3.0f, 4.0f}), "(1, 2, 3, 4)");
    EXPECT_EQ(zp::math::to_string(zp::math::quat{0.0f, 1.0f, 0.0f, 0.0f}), "(0, 1, 0, 0)");
    EXPECT_EQ(zp::math::to_string(zp::math::mat3{}), "((1, 0, 0) - (0, 1, 0) - (0, 0, 1))");
    EXPECT_EQ(zp::math::to_string(zp::math::mat4{}), "((1, 0, 0, 0) - (0, 1, 0, 0) - (0, 0, 1, 0) - (0, 0, 0, 1))");
    EXPECT_EQ(
        zp::math::to_string(
            zp::math::bb2{
                {0.0f, 0.0f},
                {1.0f, 1.0f}
    }
        ),
        "((0, 0) - (1, 1))"
    );
    EXPECT_EQ(
        zp::math::to_string(
            zp::math::bb3{
                {0.0f, 0.0f, 0.0f},
                {1.0f, 1.0f, 1.0f}
    }
        ),
        "((0, 0, 0) - (1, 1, 1))"
    );
    EXPECT_EQ(
        zp::math::to_string(
            zp::math::bb4{
                {0.0f, 0.0f, 0.0f, 0.0f},
                {1.0f, 1.0f, 1.0f, 1.0f}
    }
        ),
        "((0, 0, 0, 0) - (1, 1, 1, 1))"
    );
    EXPECT_EQ(zp::math::to_string(zp::math::ivec2{1, -2}), "(1, -2)");
    EXPECT_EQ(zp::math::to_string(zp::math::ivec3{1, 2, 3}), "(1, 2, 3)");
    EXPECT_EQ(zp::math::to_string(zp::math::ivec4{1, 2, 3, 4}), "(1, 2, 3, 4)");
}

// =========================================================================================================================================
// =========================================================================================================================================
// AxisConstantsFormRightHandedSystem: Verifies RIGHT, FORWARD, and UP define a proper right-handed basis.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMiscTest, AxisConstantsFormRightHandedSystem)
{
    using namespace zp::math;

    const vec3 right   = RIGHT;
    const vec3 forward = FORWARD;
    const vec3 up      = UP;

    EXPECT_FLOAT_EQ(dot(right, forward), 0.0f);
    EXPECT_FLOAT_EQ(dot(right, up), 0.0f);
    EXPECT_FLOAT_EQ(dot(forward, up), 0.0f);

    const vec3 right_from_cross = cross(forward, up);
    const vec3 up_from_cross    = cross(right, forward);
    const float triple_product  = dot(right, cross(forward, up));

    EXPECT_FLOAT_EQ(right_from_cross.x, RIGHT.x);
    EXPECT_FLOAT_EQ(right_from_cross.y, RIGHT.y);
    EXPECT_FLOAT_EQ(right_from_cross.z, RIGHT.z);

    EXPECT_FLOAT_EQ(up_from_cross.x, UP.x);
    EXPECT_FLOAT_EQ(up_from_cross.y, UP.y);
    EXPECT_FLOAT_EQ(up_from_cross.z, UP.z);

    EXPECT_FLOAT_EQ(triple_product, 1.0f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// CartesianPolarRoundTrip: Ensures CartesianToPolar() and PolarToCartesian() invert each other.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMiscTest, CartesianPolarRoundTrip)
{
    zp::math::vec3 original{2.0f, -2.0f, 3.0f};
    zp::math::SphericalCoords polar = zp::math::CartesianToPolar(original);
    zp::math::vec3 reconstructed    = zp::math::PolarToCartesian(polar);

    EXPECT_NEAR(reconstructed.x, original.x, 1e-5f);
    EXPECT_NEAR(reconstructed.y, original.y, 1e-5f);
    EXPECT_NEAR(reconstructed.z, original.z, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// AreParallelDetectsParallelVectors: Verifies areParallel() returns true for colinear vectors.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMiscTest, AreParallelDetectsParallelVectors)
{
    EXPECT_TRUE(zp::math::areParallel({1.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}));
    EXPECT_FALSE(zp::math::areParallel({1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
}

// =========================================================================================================================================
// =========================================================================================================================================
// RayAabbIntersectionReturnsDistance: Ensures checkRayAABBIntersection provides nearest intersection distance.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMiscTest, RayAabbIntersectionReturnsDistance)
{
    zp::math::vec3 ray_origin{0.0f, 0.0f, -5.0f};
    zp::math::vec3 ray_dir{0.0f, 0.0f, 1.0f};
    zp::math::IntersectionResult hit = zp::math::checkRayAABBIntersection(ray_origin, ray_dir, {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});

    EXPECT_TRUE(hit.hit);
    EXPECT_NEAR(hit.distance, 4.0f, 1e-5f);

    zp::math::IntersectionResult miss = zp::math::checkRayAABBIntersection(ray_origin, ray_dir, {2.0f, 2.0f, 2.0f}, {3.0f, 3.0f, 3.0f});
    EXPECT_FALSE(miss.hit);
}
