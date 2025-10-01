#include <gtest/gtest.h>
#include "zp_cpp/math.hpp"

#include <cmath>

namespace
{
    // =========================================================================================================================================
    // =========================================================================================================================================
    // rotate_vec_by_quat: Helper to rotate a vector by a quaternion using q * v * conj(q).
    // =========================================================================================================================================
    // =========================================================================================================================================
    zp::math::vec3 rotate_vec_by_quat(const zp::math::quat& q, const zp::math::vec3& v)
    {
        const zp::math::quat vec_quat{v.x, v.y, v.z, 0.0f};
        const zp::math::quat conj{-q.x, -q.y, -q.z, q.w};
        zp::math::quat rotated = q * vec_quat * conj;
        return {rotated.x, rotated.y, rotated.z};
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// NormalizeReturnsUnitQuaternion: Ensures normalize() outputs a unit quaternion preserving direction.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathQuatTest, NormalizeReturnsUnitQuaternion)
{
    zp::math::quat q{2.0f, 0.0f, 0.0f, 0.0f};
    zp::math::quat normalized = zp::math::normalize(q);

    EXPECT_NEAR(std::sqrt(normalized.x * normalized.x + normalized.w * normalized.w), 1.0f, 1e-6f);
    EXPECT_NEAR(normalized.x, 1.0f, 1e-6f);
    EXPECT_NEAR(normalized.w, 0.0f, 1e-6f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// RotationToFaceAlignsForward: Validates rotation_to_face() aligns FORWARD to the requested direction.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathQuatTest, RotationToFaceAlignsForward)
{
    zp::math::vec3 target{1.0f, 0.0f, 0.0f};
    zp::math::quat rot             = zp::math::rotation_to_face(target);

    zp::math::vec3 rotated_forward = rotate_vec_by_quat(rot, zp::math::FORWARD);

    EXPECT_NEAR(rotated_forward.x, 1.0f, 1e-5f);
    EXPECT_NEAR(rotated_forward.y, 0.0f, 1e-5f);
    EXPECT_NEAR(rotated_forward.z, 0.0f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// SlerpInterpolatesHalfway: Confirms slerp() produces the half-angle rotation between two quaternions.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathQuatTest, SlerpInterpolatesHalfway)
{
    const float angle = static_cast<float>(M_PI_2);
    zp::math::quat start{0.0f, 0.0f, 0.0f, 1.0f};
    zp::math::vec3 axis{0.0f, 0.0f, 1.0f};

    const float half_angle = angle * 0.5f;
    zp::math::quat end{axis.x * std::sin(angle / 2.0f), axis.y * std::sin(angle / 2.0f), axis.z * std::sin(angle / 2.0f), std::cos(angle / 2.0f)};

    zp::math::quat halfway = zp::math::slerp(start, end, 0.5f);

    EXPECT_NEAR(halfway.w, std::cos(half_angle / 2.0f), 1e-5f);
    EXPECT_NEAR(halfway.z, std::sin(half_angle / 2.0f), 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ProportionComputesFraction: Ensures proportion() performs floating-point division correctly.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathQuatTest, ProportionComputesFraction)
{
    EXPECT_NEAR(zp::math::proportion(2, 8), 0.25f, 1e-6f);
}
