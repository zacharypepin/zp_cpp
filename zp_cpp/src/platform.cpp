#include "zp_cpp/platform.hpp"

#include <chrono>

using namespace zp::platform;

// =========================================================================================================================================
// =========================================================================================================================================
// key_callback: Translates GLFW keyboard events into zp platform keyboard state changes and callbacks.
// =========================================================================================================================================
// =========================================================================================================================================
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    Instance* p_inst = static_cast<Instance*>(glfwGetWindowUserPointer(window));
    static input::State::Keyboard previous_keyboard_state;

    input::State::Keyboard current_keyboard_state = p_inst->input.keyboard;

    // ====================================================================================================
    // ====================================================================================================
    // Update the tracked key state for WASD and shift modifiers.
    // ====================================================================================================
    // ====================================================================================================
    {
        bool* key_slot = nullptr;
        if (key == GLFW_KEY_W)
        {
            key_slot = &current_keyboard_state.isWPressed;
        }
        else if (key == GLFW_KEY_A)
        {
            key_slot = &current_keyboard_state.isAPressed;
        }
        else if (key == GLFW_KEY_S)
        {
            key_slot = &current_keyboard_state.isSPressed;
        }
        else if (key == GLFW_KEY_D)
        {
            key_slot = &current_keyboard_state.isDPressed;
        }
        else if (key == GLFW_KEY_LEFT_SHIFT)
        {
            key_slot = &current_keyboard_state.isShiftPressed;
        }

        if (key_slot != nullptr)
        {
            if (action == GLFW_PRESS)
            {
                *key_slot = true;
            }
            else if (action == GLFW_RELEASE)
            {
                *key_slot = false;
            }

            if (previous_keyboard_state.isShiftPressed != current_keyboard_state.isShiftPressed || previous_keyboard_state.isWPressed != current_keyboard_state.isWPressed || previous_keyboard_state.isAPressed != current_keyboard_state.isAPressed
                || previous_keyboard_state.isSPressed != current_keyboard_state.isSPressed || previous_keyboard_state.isDPressed != current_keyboard_state.isDPressed)
            {
                p_inst->input.keyboard = current_keyboard_state;
                p_inst->input.on_keyboard_state_changed.trigger(zp::Void{});
            }
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // Broadcast per-key events for hotkeys that should fire on press or repeat.
    // ====================================================================================================
    // ====================================================================================================
    {
        const bool is_press_or_repeat = action == GLFW_PRESS || action == GLFW_REPEAT;
        if (!is_press_or_repeat)
        {
            previous_keyboard_state = current_keyboard_state;
            return;
        }

        if (key == GLFW_KEY_GRAVE_ACCENT)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::Backtick});
        }
        else if (key == GLFW_KEY_T)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::T});
        }
        else if (key == GLFW_KEY_Z)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::Z});
        }
        else if (key == GLFW_KEY_X)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::X});
        }
        else if (key == GLFW_KEY_SPACE)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::Space});
        }
        else if (key == GLFW_KEY_ENTER)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::Enter});
        }
        else if (key == GLFW_KEY_BACKSPACE)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::Backspace});
        }
        else if (key == GLFW_KEY_ESCAPE)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::Escape});
        }
        else if (key == GLFW_KEY_F)
        {
            p_inst->input.on_key_down.trigger(input::KeyDownEvt{input::Keys::F});
        }

        const bool did_state_change = previous_keyboard_state.isShiftPressed != current_keyboard_state.isShiftPressed || previous_keyboard_state.isWPressed != current_keyboard_state.isWPressed || previous_keyboard_state.isAPressed != current_keyboard_state.isAPressed
                                   || previous_keyboard_state.isSPressed != current_keyboard_state.isSPressed || previous_keyboard_state.isDPressed != current_keyboard_state.isDPressed;

        if (did_state_change)
        {
            zp::math::vec2 axis = {0.0f, 0.0f};
            if (current_keyboard_state.isDPressed)
            {
                axis.x += 1.0f;
            }
            if (current_keyboard_state.isAPressed)
            {
                axis.x -= 1.0f;
            }
            if (current_keyboard_state.isWPressed)
            {
                axis.y += 1.0f;
            }
            if (current_keyboard_state.isSPressed)
            {
                axis.y -= 1.0f;
            }

            if (axis.x != 0.0f || axis.y != 0.0f)
            {
                axis = zp::math::normalize(axis);
            }

            p_inst->input.on_wasd_axis_changed.trigger(input::WasdAxisChangedEvt{axis});
        }

        previous_keyboard_state = current_keyboard_state;
        p_inst->input.keyboard  = current_keyboard_state;
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// cursor_position_callback: Updates mouse position state and emits move/drag events based on GLFW cursor input.
// =========================================================================================================================================
// =========================================================================================================================================
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Instance* p_inst     = static_cast<Instance*>(glfwGetWindowUserPointer(window));
    auto& mouse_state    = p_inst->input.mouse;
    auto& keyboard_state = p_inst->input.keyboard;

    // ====================================================================================================
    // ====================================================================================================
    // Rate-limit high frequency input to roughly 100 Hz to reduce event spam.
    // ====================================================================================================
    // ====================================================================================================
    {
        static auto last_time   = std::chrono::high_resolution_clock::now();
        const auto current_time = std::chrono::high_resolution_clock::now();
        const auto elapsed_ms   = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();
        if (elapsed_ms < 10)
        {
            return;
        }
        last_time = current_time;
    }

    // ====================================================================================================
    // ====================================================================================================
    // Convert screen coordinates to normalised window space.
    // ====================================================================================================
    // ====================================================================================================
    zp::math::vec2 new_pos;
    {
        int width  = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);

        if (width == 0 || height == 0)
        {
            return;
        }

        const zp::math::vec2 window_size = {static_cast<float>(width), static_cast<float>(height)};
        new_pos                          = zp::math::vec2{static_cast<float>(xpos), static_cast<float>(ypos)} / window_size;
    }

    const zp::math::vec2 prev_pos = mouse_state.mousePosition;
    const zp::math::vec2 delta    = new_pos - prev_pos;

    mouse_state.mousePosition     = new_pos;

    // ====================================================================================================
    // ====================================================================================================
    // Emit pointer move and optional drag events.
    // ====================================================================================================
    // ====================================================================================================
    {
        input::MouseMoveEvt move_evt{};
        move_evt.pos = new_pos;
        move_evt.d   = delta;
        p_inst->input.on_mouse_moved.trigger(move_evt);

        if (mouse_state.leftButtonPressed || mouse_state.middleButtonPressed || mouse_state.rightButtonPressed)
        {
            input::MouseDragEvt drag_evt{};
            drag_evt.pos                  = new_pos;
            drag_evt.d                    = delta;
            drag_evt.leftButtonPressed    = mouse_state.leftButtonPressed;
            drag_evt.middleButtonPressed  = mouse_state.middleButtonPressed;
            drag_evt.rightButtonPressed   = mouse_state.rightButtonPressed;
            drag_evt.isShiftButtonPressed = keyboard_state.isShiftPressed;
            p_inst->input.on_mouse_dragged.trigger(drag_evt);
        }
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// mouse_button_callback: Updates button state and broadcasts pointer button events.
// =========================================================================================================================================
// =========================================================================================================================================
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)mods;

    Instance* p_inst  = static_cast<Instance*>(glfwGetWindowUserPointer(window));
    auto& mouse_state = p_inst->input.mouse;

    // ====================================================================================================
    // ====================================================================================================
    // Handle left mouse button transitions.
    // ====================================================================================================
    // ====================================================================================================
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                mouse_state.leftButtonPressed = true;
                p_inst->input.on_pointer_down.trigger(input::PointerDownEvt{mouse_state.mousePosition});
                p_inst->input.on_pointer_clicked.trigger(input::PointerClickEvt{mouse_state.mousePosition});
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_state.leftButtonPressed = false;
                p_inst->input.on_pointer_up.trigger(input::PointerUpEvt{mouse_state.mousePosition});
            }
            return;
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // Handle right mouse button transitions.
    // ====================================================================================================
    // ====================================================================================================
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                mouse_state.rightButtonPressed = true;
                p_inst->input.on_pointer_right_clicked.trigger(input::PointerRightClickEvt{mouse_state.mousePosition});
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_state.rightButtonPressed = false;
            }
            return;
        }
    }

    // ====================================================================================================
    // ====================================================================================================
    // Handle middle mouse button transitions.
    // ====================================================================================================
    // ====================================================================================================
    {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            if (action == GLFW_PRESS)
            {
                mouse_state.middleButtonPressed = true;
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_state.middleButtonPressed = false;
            }
        }
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// scroll_callback: Emits scroll wheel delta events from GLFW callbacks.
// =========================================================================================================================================
// =========================================================================================================================================
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    Instance* p_inst = static_cast<Instance*>(glfwGetWindowUserPointer(window));
    p_inst->input.on_mouse_scrolled.trigger(input::MouseScrollEvt{static_cast<float>(yoffset)});
}

// =========================================================================================================================================
// =========================================================================================================================================
// character_callback: Emits typed character events for text input.
// =========================================================================================================================================
// =========================================================================================================================================
static void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    Instance* p_inst = static_cast<Instance*>(glfwGetWindowUserPointer(window));
    p_inst->input.on_char_typed.trigger(input::CharTypedEvt{codepoint});
}

// =========================================================================================================================================
// =========================================================================================================================================
// init: Creates a GLFW window and installs callbacks for translating platform input into zp events.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::platform::init(Instance* p_inst, int window_w, int window_h)
{
    // ====================================================================================================
    // ====================================================================================================
    // Initialise GLFW and create a window without an OpenGL context.
    // ====================================================================================================
    // ====================================================================================================
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        p_inst->window.window = glfwCreateWindow(window_w, window_h, "zp_platform", nullptr, nullptr);
    }

    // ====================================================================================================
    // ====================================================================================================
    // Register this instance and the callbacks that translate GLFW events.
    // ====================================================================================================
    // ====================================================================================================
    {
        glfwSetWindowUserPointer(p_inst->window.window, p_inst);
        glfwSetKeyCallback(p_inst->window.window, key_callback);
        glfwSetCursorPosCallback(p_inst->window.window, cursor_position_callback);
        glfwSetMouseButtonCallback(p_inst->window.window, mouse_button_callback);
        glfwSetScrollCallback(p_inst->window.window, scroll_callback);
        glfwSetCharCallback(p_inst->window.window, character_callback);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// should_close: Returns true when the underlying GLFW window has been asked to close.
// =========================================================================================================================================
// =========================================================================================================================================
bool zp::platform::should_close(Instance* p_inst)
{
    return glfwWindowShouldClose(p_inst->window.window) != 0;
}

// =========================================================================================================================================
// =========================================================================================================================================
// poll_events: Pumps GLFW's event queue so callbacks are delivered.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::platform::poll_events(Instance* p_inst)
{
    (void)p_inst;
    glfwPollEvents();
}

// =========================================================================================================================================
// =========================================================================================================================================
// cleanup: Destroys the GLFW window and tears down the GLFW runtime.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::platform::cleanup(Instance* p_inst)
{
    glfwDestroyWindow(p_inst->window.window);
    p_inst->window.window = nullptr;
    glfwTerminate();
}

// =========================================================================================================================================
// =========================================================================================================================================
// get_framebuffer_size: Retrieves the current framebuffer dimensions of the GLFW window.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::platform::get_framebuffer_size(Instance* p_inst, int* p_width, int* p_height)
{
    glfwGetFramebufferSize(p_inst->window.window, p_width, p_height);
}

// =========================================================================================================================================
// =========================================================================================================================================
// wait_for_window: Blocks until the GLFW window reports a valid framebuffer size.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::platform::wait_for_window(Instance* p_inst)
{
    // ====================================================================================================
    // ====================================================================================================
    // Poll framebuffer size until a non-zero surface is reported.
    // ====================================================================================================
    // ====================================================================================================
    {
        int width  = 0;
        int height = 0;
        do {
            glfwGetFramebufferSize(p_inst->window.window, &width, &height);
            if (width == 0 || height == 0)
            {
                glfwWaitEvents();
            }
        } while (width == 0 || height == 0);
    }
}

// =========================================================================================================================================
// =========================================================================================================================================
// set_window_title: Updates the platform window title string.
// =========================================================================================================================================
// =========================================================================================================================================
void zp::platform::set_window_title(Instance* p_inst, std::string title)
{
    glfwSetWindowTitle(p_inst->window.window, title.c_str());
}
