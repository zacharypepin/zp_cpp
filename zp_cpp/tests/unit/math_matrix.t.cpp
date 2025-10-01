#include <gtest/gtest.h>
#include "zp_cpp/math.hpp"

#include <array>
#include <cmath>
#include <numbers>
#include <cstring>

// =========================================================================================================================================
// =========================================================================================================================================
// TranslateProducesExpectedMatrix: Validates translate() embeds translation in the last column.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, TranslateProducesExpectedMatrix)
{
    zp::math::vec3 offset{2.0f, -3.0f, 4.0f};
    zp::math::mat4 translate_mat = zp::math::translate(offset);

    EXPECT_FLOAT_EQ(translate_mat.f[0][3], offset.x);
    EXPECT_FLOAT_EQ(translate_mat.f[1][3], offset.y);
    EXPECT_FLOAT_EQ(translate_mat.f[2][3], offset.z);
    EXPECT_FLOAT_EQ(translate_mat.f[3][3], 1.0f);

    std::array<float, 16> actual{};
    std::memcpy(actual.data(), translate_mat.f.data(), actual.size() * sizeof(float));
    std::array<float, 16> expected{1.0f, 0.0f, 0.0f, offset.x, 0.0f, 1.0f, 0.0f, offset.y, 0.0f, 0.0f, 1.0f, offset.z, 0.0f, 0.0f, 0.0f, 1.0f};
    for (size_t i = 0; i < actual.size(); ++i)
    {
        EXPECT_FLOAT_EQ(actual[i], expected[i]);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// Mat4ConstructorStoresRowsContiguously: Confirms mat4 row ctor keeps elements in row-major order in memory.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, Mat4ConstructorStoresRowsContiguously)
{
    zp::math::mat4 m{
        zp::math::vec4{ 1.0f,  2.0f,  3.0f,  4.0f},
        zp::math::vec4{ 5.0f,  6.0f,  7.0f,  8.0f},
        zp::math::vec4{ 9.0f, 10.0f, 11.0f, 12.0f},
        zp::math::vec4{13.0f, 14.0f, 15.0f, 16.0f}
    };

    std::array<float, 16> actual{};
    std::memcpy(actual.data(), m.f.data(), actual.size() * sizeof(float));
    std::array<float, 16> expected{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};

    for (size_t i = 0; i < actual.size(); ++i)
    {
        EXPECT_FLOAT_EQ(actual[i], expected[i]);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// Mat3ConstructorStoresRowsContiguously: Confirms mat3 row ctor also uses row-major storage order.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, Mat3ConstructorStoresRowsContiguously)
{
    zp::math::mat3 m{
        zp::math::vec3{1.0f, 2.0f, 3.0f},
        zp::math::vec3{4.0f, 5.0f, 6.0f},
        zp::math::vec3{7.0f, 8.0f, 9.0f}
    };

    std::array<float, 9> actual{};
    std::memcpy(actual.data(), m.f.data(), actual.size() * sizeof(float));
    std::array<float, 9> expected{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};

    for (size_t i = 0; i < actual.size(); ++i)
    {
        EXPECT_FLOAT_EQ(actual[i], expected[i]);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// ScaleDiagonalContainsFactors: Confirms scale() places scale factors along the diagonal.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, ScaleDiagonalContainsFactors)
{
    zp::math::vec3 scale_vec{1.5f, 2.0f, -3.0f};
    zp::math::mat4 scale_mat = zp::math::scale(scale_vec);

    EXPECT_FLOAT_EQ(scale_mat.f[0][0], scale_vec.x);
    EXPECT_FLOAT_EQ(scale_mat.f[1][1], scale_vec.y);
    EXPECT_FLOAT_EQ(scale_mat.f[2][2], scale_vec.z);
    EXPECT_FLOAT_EQ(scale_mat.f[3][3], 1.0f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// RotateXAppliesCorrectRotation: Ensures rotate_x() rotates a unit Y vector onto Z.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, RotateXAppliesCorrectRotation)
{
    const float quarter_turn = std::numbers::pi_v<float> * 0.5f;
    zp::math::mat4 rotate    = zp::math::rotate_x(quarter_turn);

    zp::math::vec4 y_axis{0.0f, 1.0f, 0.0f, 1.0f};
    zp::math::vec4 rotated = rotate * y_axis;

    EXPECT_NEAR(rotated.y, 0.0f, 1e-5f);
    EXPECT_NEAR(rotated.z, 1.0f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// RotateZSpinsXAxisToYAxis: Verifies rotate_z() maps the X axis to Y for ninety degrees.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, RotateZSpinsXAxisToYAxis)
{
    const float quarter_turn = std::numbers::pi_v<float> * 0.5f;
    zp::math::mat4 rotate    = zp::math::rotate_z(quarter_turn);

    zp::math::vec4 x_axis{1.0f, 0.0f, 0.0f, 1.0f};
    zp::math::vec4 rotated = rotate * x_axis;

    EXPECT_NEAR(rotated.x, 0.0f, 1e-5f);
    EXPECT_NEAR(rotated.y, 1.0f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// LookAtProducesRightHandedBasis: Confirms look_at() matches the +X/+Y/+Z axis convention.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, LookAtProducesRightHandedBasis)
{
    const zp::math::vec3 eye    = {0.0f, 0.0f, 0.0f};
    const zp::math::vec3 center = eye + zp::math::FORWARD;
    const zp::math::vec3 up     = zp::math::UP;

    zp::math::mat4 view         = zp::math::look_at(eye, center, up);

    zp::math::vec3 right_basis{view.f[0][0], view.f[0][1], view.f[0][2]};
    zp::math::vec3 up_basis{view.f[1][0], view.f[1][1], view.f[1][2]};
    zp::math::vec3 forward_basis{-view.f[2][0], -view.f[2][1], -view.f[2][2]};

    right_basis   = zp::math::normalize(right_basis);
    up_basis      = zp::math::normalize(up_basis);
    forward_basis = zp::math::normalize(forward_basis);

    EXPECT_NEAR(zp::math::dot(right_basis, zp::math::RIGHT), 1.0f, 1e-5f);
    EXPECT_NEAR(zp::math::dot(up_basis, zp::math::UP), 1.0f, 1e-5f);
    EXPECT_NEAR(zp::math::dot(forward_basis, zp::math::FORWARD), 1.0f, 1e-5f);

    zp::math::vec3 cross_check = zp::math::normalize(zp::math::cross(forward_basis, up_basis));
    EXPECT_NEAR(zp::math::dot(cross_check, zp::math::RIGHT), 1.0f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// PerspectiveSetsFovAndPlanes: Validates key elements of the perspective projection matrix.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, PerspectiveSetsFovAndPlanes)
{
    const float fov        = std::numbers::pi_v<float> * 0.25f;
    const float aspect     = 16.0f / 9.0f;
    const float near_plane = 0.1f;
    const float far_plane  = 100.0f;

    zp::math::mat4 proj    = zp::math::perspective(fov, aspect, near_plane, far_plane);

    EXPECT_NEAR(proj.f[0][0], 1.0f / (aspect * std::tan(fov / 2.0f)), 1e-5f);
    EXPECT_NEAR(proj.f[1][1], 1.0f / std::tan(fov / 2.0f), 1e-5f);
    EXPECT_NEAR(proj.f[2][2], -(far_plane + near_plane) / (far_plane - near_plane), 1e-5f);
    EXPECT_NEAR(proj.f[2][3], -(2.0f * far_plane * near_plane) / (far_plane - near_plane), 1e-5f);
    EXPECT_NEAR(proj.f[3][2], -1.0f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// OrthoCreatesUnitCube: Checks ortho() output for a symmetric unit cube volume.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathMatrixTest, OrthoCreatesUnitCube)
{
    zp::math::mat4 proj = zp::math::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 2.0f);

    EXPECT_NEAR(proj.f[0][0], 1.0f, 1e-5f);
    EXPECT_NEAR(proj.f[1][1], 1.0f, 1e-5f);
    EXPECT_NEAR(proj.f[2][2], -1.0f, 1e-5f);
    EXPECT_NEAR(proj.f[3][3], 1.0f, 1e-5f);
}
