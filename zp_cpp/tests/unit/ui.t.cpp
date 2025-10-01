#include <gtest/gtest.h>

#include "zp_cpp/ui.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace
{
    struct UiTestFixture
    {
        zp::ui::Instance instance;
        std::unordered_map<zp::uuid::uuid, zp::ui::FontData> fonts;
        std::vector<zp::ui::Elem> elems;
        zp::span<zp::ui::Elem> elem_span{};

        UiTestFixture()
        {
            zp::ui::init(&instance);
            instance.config.p_fonts = &fonts;
            instance.config.p_elems = &elem_span;
        }
    };
}

// =========================================================================================================================================
// =========================================================================================================================================
// InitAllocatesInternalState: Validates init() prepares the instance with internal bookkeeping.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(UIUnitTest, InitAllocatesInternalState)
{
    zp::ui::Instance instance{};
    zp::ui::init(&instance);

    EXPECT_NE(instance.p_i, nullptr);
}

// =========================================================================================================================================
// =========================================================================================================================================
// UpdateResolvesAbsoluteChild: Ensures update() resolves a static absolute child into normalised geometry.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(UIUnitTest, UpdateResolvesAbsoluteChild)
{
    UiTestFixture fixture;

    fixture.instance.config.root_width  = 200.0f;
    fixture.instance.config.root_height = 100.0f;

    zp::ui::Elem root{};
    root.wh     = {200.0f, 100.0f};
    root.bg_col = {0.2f, 0.2f, 0.2f, 1.0f};

    zp::ui::Elem child{};
    child.parent_idx  = 0;
    child.pos_type    = zp::ui::PosType::Absolute;
    child.xy_type     = zp::ui::XYType::Abs;
    child.xy          = {10.0f, 20.0f};
    child.wh_type     = zp::ui::WHType::Abs;
    child.wh          = {50.0f, 40.0f};
    child.bg_col      = {0.8f, 0.1f, 0.1f, 1.0f};

    fixture.elems     = {root, child};
    fixture.elem_span = {fixture.elems.data(), fixture.elems.size()};

    zp::ui::update(&fixture.instance);

    ASSERT_FALSE(fixture.instance.output.empty());

    auto child_it = std::find_if(fixture.instance.output.begin(), fixture.instance.output.end(), [](const zp::ui::Resolved& resolved) { return resolved.elem_idx == 1 && !resolved.text_char.has_value(); });

    ASSERT_NE(child_it, fixture.instance.output.end());

    const float expected_pos_x   = child.xy.x / fixture.instance.config.root_width;
    const float expected_pos_y   = child.xy.y / fixture.instance.config.root_height;
    const float expected_scale_x = child.wh.x / fixture.instance.config.root_width;
    const float expected_scale_y = child.wh.y / fixture.instance.config.root_height;

    EXPECT_FLOAT_EQ(child_it->pos.x, expected_pos_x);
    EXPECT_FLOAT_EQ(child_it->pos.y, expected_pos_y);
    EXPECT_FLOAT_EQ(child_it->scale.x, expected_scale_x);
    EXPECT_FLOAT_EQ(child_it->scale.y, expected_scale_y);
    EXPECT_FALSE(child_it->bg_img.has_value());
}

// =========================================================================================================================================
// =========================================================================================================================================
// PointInsideQueries: Verifies is_point_inside() and calc_point_inside() respond with expected hit-test results.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(UIUnitTest, PointInsideQueries)
{
    UiTestFixture fixture;

    fixture.instance.config.root_width  = 200.0f;
    fixture.instance.config.root_height = 100.0f;

    zp::ui::Elem root{};
    root.wh = {200.0f, 100.0f};

    zp::ui::Elem child{};
    child.parent_idx  = 0;
    child.pos_type    = zp::ui::PosType::Absolute;
    child.xy_type     = zp::ui::XYType::Abs;
    child.xy          = {10.0f, 20.0f};
    child.wh_type     = zp::ui::WHType::Abs;
    child.wh          = {50.0f, 40.0f};

    fixture.elems     = {root, child};
    fixture.elem_span = {fixture.elems.data(), fixture.elems.size()};

    zp::ui::update(&fixture.instance);

    zp::math::vec2 inside{0.10f, 0.30f};
    zp::math::vec2 outside{0.90f, 0.10f};

    EXPECT_TRUE(zp::ui::is_point_inside(&fixture.instance, 1, inside));
    EXPECT_FALSE(zp::ui::is_point_inside(&fixture.instance, 1, outside));

    zp::math::vec2 relative{};
    ASSERT_TRUE(zp::ui::calc_point_inside(&fixture.instance, 1, inside, &relative));
    const float expected_pos_x   = child.xy.x / fixture.instance.config.root_width;
    const float expected_pos_y   = child.xy.y / fixture.instance.config.root_height;
    const float expected_scale_x = child.wh.x / fixture.instance.config.root_width;
    const float expected_scale_y = child.wh.y / fixture.instance.config.root_height;

    EXPECT_FLOAT_EQ(relative.x, (inside.x - expected_pos_x) / expected_scale_x);
    EXPECT_FLOAT_EQ(relative.y, (inside.y - expected_pos_y) / expected_scale_y);

    EXPECT_FALSE(zp::ui::calc_point_inside(&fixture.instance, 1, outside, &relative));
}
