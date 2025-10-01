#include "zp_cpp/math.hpp"

using namespace zp::math;
using namespace zp::math::splines;

// ========================================================================================================================================
// ========================================================================================================================================
// bezierPoint: Evaluates a cubic Bézier curve at parameter t using four control points.
// ========================================================================================================================================
// ========================================================================================================================================
static zp::math::vec2 bezierPoint(std::array<vec2, 4> cp, float t)
{
    float u  = 1.0f - t;
    float b0 = u * u * u;
    float b1 = 3 * u * u * t;
    float b2 = 3 * u * t * t;
    float b3 = t * t * t;
    return cp[0] * b0 + cp[1] * b1 + cp[2] * b2 + cp[3] * b3;
}

// ========================================================================================================================================
// ========================================================================================================================================
// bezierTangent: Computes the derivative of a cubic Bézier curve at parameter t.
// ========================================================================================================================================
// ========================================================================================================================================
static zp::math::vec2 bezierTangent(std::array<vec2, 4> cp, float t)
{
    float u              = 1.0f - t;
    zp::math::vec2 term1 = (cp[1] - cp[0]) * (3 * u * u);
    zp::math::vec2 term2 = (cp[2] - cp[1]) * (6 * u * t);
    zp::math::vec2 term3 = (cp[3] - cp[2]) * (3 * t * t);
    return term1 + term2 + term3;
}

// ========================================================================================================================================
// ========================================================================================================================================
// sample_bezier_tris: Tessellates a cubic Bézier stroke into triangle pairs for rendering.
// ========================================================================================================================================
// ========================================================================================================================================
std::vector<zp::math::vec2> splines::sample_bezier_tris(vec2 screen_size, std::array<vec2, 4> cp, int segments, float width_pixels)
{
    std::vector<zp::math::vec2> triangles;
    std::vector<zp::math::vec2> left, right;
    zp::math::vec2 last_normal_px = {0.0f, 1.0f}; // fallback if tangent degenerate

    for (int i = 0; i <= segments; ++i)
    {
        float t                   = (float)i / segments;
        zp::math::vec2 p          = bezierPoint(cp, t);

        // Compute tangent, move to pixel space
        zp::math::vec2 tangent    = bezierTangent(cp, t);
        zp::math::vec2 tangent_px = {tangent.x * screen_size.x, tangent.y * screen_size.y};

        // Perpendicular in pixel space
        zp::math::vec2 normal_px  = {-tangent_px.y, tangent_px.x};
        float len_px              = std::sqrt(normal_px.x * normal_px.x + normal_px.y * normal_px.y);
        if (len_px != 0.0f)
        {
            normal_px.x    /= len_px;
            normal_px.y    /= len_px;
            last_normal_px  = normal_px;
        }
        else
        {
            normal_px = last_normal_px;
        }

        // Offset in pixel space, then convert back to normalized
        zp::math::vec2 offset_px = {normal_px.x * (width_pixels * 0.5f), normal_px.y * (width_pixels * 0.5f)};
        zp::math::vec2 offset    = {offset_px.x / screen_size.x, offset_px.y / screen_size.y};

        left.push_back({p.x - offset.x, p.y - offset.y});
        right.push_back({p.x + offset.x, p.y + offset.y});
    }

    for (int i = 0; i < segments; ++i)
    {
        triangles.push_back(left[i]);
        triangles.push_back(right[i]);
        triangles.push_back(right[i + 1]);
        triangles.push_back(left[i]);
        triangles.push_back(right[i + 1]);
        triangles.push_back(left[i + 1]);
    }

    return triangles;
}
