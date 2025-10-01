#pragma once

#include "zp_cpp/events.hpp"
#include "zp_cpp/math.hpp"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace zp::platform
{
    namespace window
    {
        struct State
        {
            GLFWwindow* window = nullptr;
        };
    }

    namespace input
    {
        struct MouseMoveEvt
        {
            zp::math::vec2 pos;
            zp::math::vec2 d;
        };

        struct PointerEvt
        {
            zp::math::vec2 pos;
        };
        using PointerDownEvt       = PointerEvt;
        using PointerUpEvt         = PointerEvt;
        using PointerClickEvt      = PointerEvt;
        using PointerRightClickEvt = PointerEvt;

        struct MouseDragEvt
        {
            zp::math::vec2 pos;
            zp::math::vec2 d;
            bool leftButtonPressed;
            bool middleButtonPressed;
            bool rightButtonPressed;
            bool isShiftButtonPressed;
        };

        struct MouseScrollEvt
        {
            float d;
        };

        enum class Keys
        {
            Backtick,
            T,
            Z,
            X,
            Space,
            Enter,
            Backspace,
            Escape,
            F
        };

        struct KeyDownEvt
        {
            Keys key;
        };

        struct WasdAxisChangedEvt
        {
            zp::math::vec2 axis;
        };

        struct CharTypedEvt
        {
            std::uint32_t unicode;
        };

        struct State
        {
            struct Keyboard
            {
                bool isShiftPressed = false;
                bool isWPressed     = false;
                bool isAPressed     = false;
                bool isSPressed     = false;
                bool isDPressed     = false;
            };
            Keyboard keyboard;

            struct Mouse
            {
                zp::math::vec2 mousePosition = {0.0f, 0.0f};
                bool leftButtonPressed       = false;
                bool middleButtonPressed     = false;
                bool rightButtonPressed      = false;
            };
            Mouse mouse;

            zp::Event<MouseMoveEvt> on_mouse_moved;
            zp::Event<PointerDownEvt> on_pointer_down;
            zp::Event<PointerUpEvt> on_pointer_up;
            zp::Event<PointerClickEvt> on_pointer_clicked;
            zp::Event<PointerRightClickEvt> on_pointer_right_clicked;
            zp::Event<MouseDragEvt> on_mouse_dragged;
            zp::Event<MouseScrollEvt> on_mouse_scrolled;
            zp::Event<KeyDownEvt> on_key_down;
            zp::Event<WasdAxisChangedEvt> on_wasd_axis_changed;
            zp::VoidEvent on_keyboard_state_changed;
            zp::Event<CharTypedEvt> on_char_typed;
        };
    }

    struct Instance
    {
        window::State window;
        input::State input;
    };

    // =========================================================================================================================================
    // =========================================================================================================================================
    // init: Creates a GLFW window and installs callbacks for translating platform input into zp events.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void init(Instance* p_inst, int window_w, int window_h);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // should_close: Returns true when the underlying GLFW window has been asked to close.
    // =========================================================================================================================================
    // =========================================================================================================================================
    bool should_close(Instance* p_inst);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // poll_events: Pumps GLFW's event queue so callbacks are delivered.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void poll_events(Instance* p_inst);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // cleanup: Destroys the GLFW window and tears down the GLFW runtime.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void cleanup(Instance* p_inst);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // get_framebuffer_size: Retrieves the current framebuffer dimensions of the GLFW window.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void get_framebuffer_size(Instance* p_inst, int* p_width, int* p_height);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // wait_for_window: Blocks until the GLFW window reports a valid framebuffer size.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void wait_for_window(Instance* p_inst);

    // =========================================================================================================================================
    // =========================================================================================================================================
    // set_window_title: Updates the platform window title string.
    // =========================================================================================================================================
    // =========================================================================================================================================
    void set_window_title(Instance* p_inst, std::string title);
}
