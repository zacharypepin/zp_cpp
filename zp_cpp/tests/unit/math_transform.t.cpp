#include <gtest/gtest.h>
#include "zp_cpp/math.hpp"

#include <cmath>
#include <numbers>

namespace
{
    // =========================================================================================================================================
    // =========================================================================================================================================
    // apply_transform: Applies a mat4 to a vec3 treating w as 1 for position vectors.
    // =========================================================================================================================================
    // =========================================================================================================================================
    zp::math::vec3 apply_transform(const zp::math::mat4& m, const zp::math::vec3& v)
    {
        zp::math::vec4 extended{v.x, v.y, v.z, 1.0f};
        zp::math::vec4 result = m * extended;
        return {result.x, result.y, result.z};
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// TransformMatCombinesTRS: Validates get_transform_mat() composes translation, rotation, and scale correctly.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathTransformTest, TransformMatCombinesTRS)
{
    zp::math::vec3 translate{1.0f, 2.0f, 3.0f};
    const float half_angle = std::numbers::pi_v<float> * 0.25f;
    zp::math::quat rot{0.0f, 0.0f, std::sin(half_angle), std::cos(half_angle)}; // 90 deg about Z
    zp::math::vec3 scale{2.0f, 3.0f, 4.0f};

    zp::math::mat4 model = zp::math::get_transform_mat(translate, rot, scale);
    zp::math::vec3 point{1.0f, 0.0f, 0.0f};
    zp::math::vec3 transformed = apply_transform(model, point);

    EXPECT_NEAR(transformed.x, translate.x, 1e-5f);
    EXPECT_NEAR(transformed.y, translate.y + scale.x, 1e-5f);
    EXPECT_NEAR(transformed.z, translate.z, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// TransformEulerOverloadRespectsAngles: Ensures Euler overload rotates around Z for simple yaw input.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathTransformTest, TransformEulerOverloadRespectsAngles)
{
    zp::math::vec3 translate{0.0f, 0.0f, 0.0f};
    zp::math::vec3 euler{0.0f, 0.0f, std::numbers::pi_v<float> * 0.5f};
    zp::math::vec3 scale{1.0f, 1.0f, 1.0f};

    zp::math::mat4 model = zp::math::get_transform_mat(translate, euler, scale);
    zp::math::vec3 point{1.0f, 0.0f, 0.0f};
    zp::math::vec3 transformed = apply_transform(model, point);

    EXPECT_NEAR(transformed.x, 0.0f, 1e-5f);
    EXPECT_NEAR(transformed.y, 1.0f, 1e-5f);
    EXPECT_NEAR(transformed.z, 0.0f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ModelToNormalMatInvertsTranspose: Confirms model_to_nrm_mat() returns transpose inverse upper-left block.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathTransformTest, ModelToNormalMatInvertsTranspose)
{
    zp::math::mat4 model  = zp::math::scale({2.0f, 3.0f, 4.0f});
    zp::math::mat3 normal = zp::math::model_to_nrm_mat(model);

    EXPECT_NEAR(normal.f[0][0], 0.5f, 1e-5f);
    EXPECT_NEAR(normal.f[1][1], 1.0f / 3.0f, 1e-5f);
    EXPECT_NEAR(normal.f[2][2], 0.25f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// Vec2ToIvec2RoundsComponents: Validates vec2_to_ivec2() rounds to nearest integer.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathTransformTest, Vec2ToIvec2RoundsComponents)
{
    zp::math::vec2 v{1.4f, -2.6f};
    zp::math::ivec2 result = zp::math::vec2_to_ivec2(v);

    EXPECT_EQ(result.x, 1);
    EXPECT_EQ(result.y, -3);
}
