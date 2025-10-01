#include <gtest/gtest.h>
#include "zp_cpp/math.hpp"

#include <cmath>
#include <numbers>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// =========================================================================================================================================
// =========================================================================================================================================
// InitSeedsCameraState: Validates init() copies configuration fields into the camera state.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathOrbitCameraTest, InitSeedsCameraState)
{
    zp::math::OrbitCamera camera;
    zp::math::OrbitCameraConfig cfg{
        {5.0f, -5.0f, 5.0f},
        {0.0f,  0.0f, 0.0f},
        60.0f,
        16.0f / 9.0f
    };
    camera.init(cfg);

    EXPECT_FLOAT_EQ(camera.position.x, 5.0f);
    EXPECT_FLOAT_EQ(camera.position.y, -5.0f);
    EXPECT_FLOAT_EQ(camera.position.z, 5.0f);
    EXPECT_FLOAT_EQ(camera.vert_fov_deg, 60.0f);
    EXPECT_FLOAT_EQ(camera.aspect_ratio, 16.0f / 9.0f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// AxesRemainOrthogonal: Ensures forward, right, and up axes stay orthonormal after initialization.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathOrbitCameraTest, AxesRemainOrthogonal)
{
    zp::math::OrbitCamera camera;
    camera.init({
        {5.0f, -5.0f, 5.0f},
        {0.0f,  0.0f, 0.0f},
        60.0f,
        1.0f
    });

    zp::math::vec3 forward = camera.get_forward_axis();
    zp::math::vec3 right   = camera.get_right_axis();
    zp::math::vec3 up      = camera.get_up_axis();

    auto dot               = [](const zp::math::vec3& a, const zp::math::vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; };

    EXPECT_NEAR(dot(forward, right), 0.0f, 1e-5f);
    EXPECT_NEAR(dot(forward, up), 0.0f, 1e-5f);
    EXPECT_NEAR(dot(right, up), 0.0f, 1e-5f);

    zp::math::vec3 derived_right = zp::math::normalize(zp::math::cross(forward, up));
    zp::math::vec3 derived_up    = zp::math::normalize(zp::math::cross(right, forward));

    EXPECT_NEAR(derived_right.x, right.x, 1e-5f);
    EXPECT_NEAR(derived_right.y, right.y, 1e-5f);
    EXPECT_NEAR(derived_right.z, right.z, 1e-5f);

    EXPECT_NEAR(derived_up.x, up.x, 1e-5f);
    EXPECT_NEAR(derived_up.y, up.y, 1e-5f);
    EXPECT_NEAR(derived_up.z, up.z, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// NearPlaneDimensionsMatchFov: Validates near plane size computations for a known configuration.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathOrbitCameraTest, NearPlaneDimensionsMatchFov)
{
    zp::math::OrbitCamera camera;
    camera.init({
        {5.0f, -5.0f, 5.0f},
        {0.0f,  0.0f, 0.0f},
        60.0f,
        2.0f
    });

    float near_height        = camera.get_near_plane_height(1.0f);
    float near_width         = camera.get_near_plane_width(near_height);

    const float half_fov_rad = (std::numbers::pi_v<float> * 60.0f / 180.0f) * 0.5f;
    EXPECT_NEAR(near_height, 2.0f * std::tan(half_fov_rad), 1e-5f);
    EXPECT_NEAR(near_width, near_height * 2.0f, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ScreenPointMappingsFollowRays: Confirms screen center projects to the near plane and ray direction matches GLM.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathOrbitCameraTest, ScreenPointMappingsFollowRays)
{
    zp::math::OrbitCamera camera;
    camera.init({
        {5.0f, -5.0f, 5.0f},
        {0.0f,  0.0f, 0.0f},
        60.0f,
        1.0f
    });

    constexpr float near_dist = 1.0f;
    constexpr float far_dist  = 10.0f;

    zp::math::vec3 near_point = camera.screen_point_to_near_world({0.5f, 0.5f}, near_dist, far_dist);
    zp::math::vec3 ray_dir    = camera.screen_point_to_ray_dir({0.5f, 0.5f}, near_dist, far_dist);

    glm::mat4 view            = glm::lookAt(glm::vec3(5.0f, -5.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 proj            = glm::perspective(std::numbers::pi_v<float> / 3.0f, 1.0f, near_dist, far_dist);
    glm::vec3 expected_near   = glm::unProject(glm::vec3(0.5f, 0.5f, 0.0f), view, proj, glm::vec4(0, 0, 1, 1));
    glm::vec3 expected_far    = glm::unProject(glm::vec3(0.5f, 0.5f, 1.0f), view, proj, glm::vec4(0, 0, 1, 1));
    glm::vec3 expected_dir    = glm::normalize(expected_far - expected_near);

    EXPECT_NEAR(near_point.x, expected_near.x, 1e-4f);
    EXPECT_NEAR(near_point.y, expected_near.y, 1e-4f);
    EXPECT_NEAR(near_point.z, expected_near.z, 1e-4f);

    EXPECT_NEAR(ray_dir.x, expected_dir.x, 1e-5f);
    EXPECT_NEAR(ray_dir.y, expected_dir.y, 1e-5f);
    EXPECT_NEAR(ray_dir.z, expected_dir.z, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// RotateOrbitMaintainsDistance: Ensures rotate() moves the camera on a sphere around the target.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathOrbitCameraTest, RotateOrbitMaintainsDistance)
{
    zp::math::OrbitCamera camera;
    camera.init({
        {5.0f, -5.0f, 5.0f},
        {0.0f,  0.0f, 0.0f},
        60.0f,
        1.0f
    });
    float original_distance = zp::math::length(camera.position - camera.target_pos);

    camera.rotate({0.25f, 0.1f}, 1.0f);

    float new_distance = zp::math::length(camera.position - camera.target_pos);
    EXPECT_NEAR(original_distance, new_distance, 1e-4f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// PanShiftsTargetAndPositionTogether: Verifies pan() translates both position and target without changing offset.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathOrbitCameraTest, PanShiftsTargetAndPositionTogether)
{
    zp::math::OrbitCamera camera;
    camera.init({
        {5.0f, -5.0f, 5.0f},
        {0.0f,  0.0f, 0.0f},
        60.0f,
        1.0f
    });
    zp::math::vec3 offset = camera.position - camera.target_pos;

    camera.pan({0.1f, -0.2f}, 2.0f);

    EXPECT_NEAR(camera.position.x - camera.target_pos.x, offset.x, 1e-5f);
    EXPECT_NEAR(camera.position.y - camera.target_pos.y, offset.y, 1e-5f);
    EXPECT_NEAR(camera.position.z - camera.target_pos.z, offset.z, 1e-5f);
}

// =========================================================================================================================================
// =========================================================================================================================================
// ZoomMovesAlongViewDirection: Confirms zoom() brings the camera closer while respecting minimum distance.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(MathOrbitCameraTest, ZoomMovesAlongViewDirection)
{
    zp::math::OrbitCamera camera;
    camera.init({
        {5.0f, -5.0f, 5.0f},
        {0.0f,  0.0f, 0.0f},
        60.0f,
        1.0f
    });

    float original_distance = zp::math::length(camera.position - camera.target_pos);

    camera.zoom(0.5f, 1.0f);
    EXPECT_LT(zp::math::length(camera.position - camera.target_pos), original_distance);

    camera.zoom(10.0f, 1.0f); // attempt to cross minimum distance from inside
    EXPECT_GT(zp::math::length(camera.position - camera.target_pos), 0.01f);
}
