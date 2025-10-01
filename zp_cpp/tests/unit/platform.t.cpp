#include <gtest/gtest.h>

#include "zp_cpp/platform.hpp"

// =========================================================================================================================================
// =========================================================================================================================================
// InstanceDefaults: Validates default constructed platform instance state.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(PlatformUnitTest, InstanceDefaults)
{
    zp::platform::Instance instance{};

    EXPECT_EQ(instance.window.window, nullptr);
    EXPECT_FALSE(instance.input.keyboard.isShiftPressed);
    EXPECT_FALSE(instance.input.keyboard.isWPressed);
    EXPECT_FALSE(instance.input.keyboard.isAPressed);
    EXPECT_FALSE(instance.input.keyboard.isSPressed);
    EXPECT_FALSE(instance.input.keyboard.isDPressed);
    EXPECT_FLOAT_EQ(instance.input.mouse.mousePosition.x, 0.0f);
    EXPECT_FLOAT_EQ(instance.input.mouse.mousePosition.y, 0.0f);
    EXPECT_FALSE(instance.input.mouse.leftButtonPressed);
    EXPECT_FALSE(instance.input.mouse.middleButtonPressed);
    EXPECT_FALSE(instance.input.mouse.rightButtonPressed);
}

// =========================================================================================================================================
// =========================================================================================================================================
// EventTriggersInvokeSubscribers: Ensures input events fire subscribed callbacks.
// =========================================================================================================================================
// =========================================================================================================================================
TEST(PlatformUnitTest, EventTriggersInvokeSubscribers)
{
    zp::platform::input::State input_state{};

    bool mouse_move_received = false;
    input_state.on_mouse_moved.subscribe([&mouse_move_received](zp::platform::input::MouseMoveEvt evt) { mouse_move_received = evt.pos.x == 0.5f && evt.pos.y == 0.25f && evt.d.x == 0.1f && evt.d.y == -0.2f; });

    const zp::platform::input::MouseMoveEvt move_evt = {
        .pos = {0.5f, 0.25f},
        .d   = {0.1f, -0.2f},
    };
    input_state.on_mouse_moved.trigger(move_evt);
    EXPECT_TRUE(mouse_move_received);

    bool void_event_received = false;
    input_state.on_keyboard_state_changed.subscribe([&void_event_received](zp::Void) { void_event_received = true; });
    input_state.on_keyboard_state_changed.trigger(zp::Void{});
    EXPECT_TRUE(void_event_received);
}
