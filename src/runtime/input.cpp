#include "include.hpp"

namespace vivianite {
    void Input::initialize() {
        auto* r_ctx = (renderer*)r_ptr;

        glfwSetKeyCallback(r_ctx->window, key_callback);
        glfwSetScrollCallback(r_ctx->window, scroll_callback);
        glfwSetMouseButtonCallback(r_ctx->window, mouse_button_callback);

        glfwSetInputMode(r_ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(r_ctx->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    void Input::key_callback(GLFWwindow* window, int key, [[maybe_unused]]int scancode, int action, int mods) {
        vivianite::renderer* r_ctx = (renderer*)glfwGetWindowUserPointer(window);
        Input* i_ctx = r_ctx->i_ctx;

        KeyEvent event {
            .key = i_ctx->glfw_to_keytype(key),
            .action =
                action == GLFW_PRESS ? KEY_DOWN :
                action == GLFW_RELEASE ? KEY_UP :
                KEY_REPEAT,
            .mods = mods
        };

        i_ctx->keys[i_ctx->glfw_to_keytype(key)] = action != GLFW_RELEASE;

        for (auto& callback : i_ctx->key_callbacks) {
            callback(event);
        }
    }

    void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        vivianite::renderer* r_ctx = (renderer*)glfwGetWindowUserPointer(window);
        Input* i_ctx = r_ctx->i_ctx;

        i_ctx->rel_mouse_scroll_x = xoffset;
        i_ctx->rel_mouse_scroll_y = yoffset;

        i_ctx->abs_mouse_scroll_x += xoffset;
        i_ctx->abs_mouse_scroll_y += yoffset;
    }

    void Input::mouse_button_callback(GLFWwindow* window, int button, int action, [[maybe_unused]]int mods) {
        vivianite::renderer* r_ctx = (renderer*)glfwGetWindowUserPointer(window);
        Input* i_ctx = r_ctx->i_ctx;

        i_ctx->keys[i_ctx->glfw_to_keytype(button)] = action != GLFW_RELEASE;
    }

    Input::~Input() {
        auto* r_ctx = (renderer*)r_ptr;
        glfwSetInputMode(r_ctx->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void Input::update() {
        glfwPollEvents();
        auto* r_ctx = (renderer*)r_ptr;

        this->last_mouse_x = this->abs_mouse_x;
        this->last_mouse_y = this->abs_mouse_y;

        for (int i = KeyType::C_JOYSTICK_1; i < KeyType::C_JOYSTICK_16 + 1; i++) {
            if (glfwJoystickPresent(keytype_to_glfw_joystick((KeyType)i)) && glfwJoystickIsGamepad(keytype_to_glfw_joystick((KeyType)i))) {
                GLFWgamepadstate state;

                if (glfwGetGamepadState(keytype_to_glfw_joystick((KeyType)i), &state)) {
                    for (int j = KeyType::C_a; j < C_dpad_left + 1; j++) {
                        int button = keytype_to_glfw_gamepad_button((KeyType)j);

                        if ((button == -1) || (button == KeyType::K_unknown))
                            continue;

                        keys[j] = state.buttons[button] != GLFW_RELEASE;
                    }

                    for (int j = KeyType::C_left_x; j < KeyType::C_right_trigger + 1; j++) {
                        if ((j != C_left_trigger) && (j != C_right_trigger)) {
                            analog_axes[j - C_left_x] = apply_deadzone(state.axes[keytype_to_glfw_gamepad_button((KeyType)j)]);
                        }
                        else {
                            analog_axes[j - C_left_x] = state.axes[keytype_to_glfw_gamepad_button((KeyType)j)];
                        }
                    }
                }
            }
        }

        glfwGetCursorPos(r_ctx->window, &this->abs_mouse_x, &this->abs_mouse_y);
        this->rel_mouse_x = this->last_mouse_x - this->abs_mouse_x;
        this->rel_mouse_y = this->last_mouse_y - this->abs_mouse_y;
    }

    bool Input::get_action(std::string action_name) {
        return this->is_pressed(this->actions[action_name]);
    }

    void Input::set_action(std::string action_name, KeyType key) {
        this->actions[action_name] = key;
    }

    float Input::get_axis(std::string name) {
        auto* r_ctx = (renderer*)r_ptr;
        auto it = input_axes.find(name);

        if (it == input_axes.end()) {
            return 0.0f;
        }

        const auto& axis = it->second;

        float joystick_axis = 0.0f;
        if (axis.analog.has_value()) {
            joystick_axis = this->analog_axes[axis.analog.value() - KeyType::C_left_x];

            if ((axis.analog.value() == C_left_y) || (axis.analog.value() == C_right_y))
                joystick_axis = -joystick_axis;
        }

        if (axis.time_scaled)
            joystick_axis *= r_ctx->delta_time * 60.0f;

        float mouse_axis = 0.0f;
        if (axis.mouse_analog.has_value()) {
            if (axis.mouse_analog.value() == KeyType::M_x) {
                mouse_axis = -this->rel_mouse_x;
            }
            else if (axis.mouse_analog.value() == KeyType::M_y) {
                mouse_axis = this->rel_mouse_y;
            }

            mouse_axis *= this->mouse_sensitivity;
        }
        float analog_axis = std::abs(joystick_axis) > std::abs(mouse_axis) ? joystick_axis : mouse_axis;

        float keyboard_axis = 0.0f;
        if (axis.keys.has_value()) {
            for (auto k: axis.keys.value()) {
                auto [positive, negative] = k;
                keyboard_axis += keys[positive] - keys[negative];
            }
        }
        keyboard_axis = std::min(keyboard_axis, 1.0f);
        keyboard_axis = std::max(keyboard_axis, -1.0f);

        return std::abs(analog_axis) > std::abs(keyboard_axis) ? analog_axis : keyboard_axis;
    }

    void Input::set_axis(std::string name, Axis axis) {
        Axis& input_axis = input_axes[name];

        input_axis = axis;
    }

    float Input::apply_deadzone(float value) {
        return std::abs(value) < deadzone ? 0.0f : value;
    }

    KeyType Input::glfw_gamepad_button_to_keytype(int button) {
        switch (button) {
            case GLFW_GAMEPAD_BUTTON_A:
                return C_a;

            case GLFW_GAMEPAD_BUTTON_B:
                return C_b;

            case GLFW_GAMEPAD_BUTTON_X:
                return C_x;

            case GLFW_GAMEPAD_BUTTON_Y:
                return C_y;

            case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER:
                return C_left_bumper;

            case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER:
                return C_right_bumper;

            case GLFW_GAMEPAD_BUTTON_BACK:
                return C_back;

            case GLFW_GAMEPAD_BUTTON_START:
                return C_start;

            case GLFW_GAMEPAD_BUTTON_GUIDE:
                return C_guide;

            case GLFW_GAMEPAD_BUTTON_LEFT_THUMB:
                return C_left_thumb;

            case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB:
                return C_right_thumb;

            case GLFW_GAMEPAD_BUTTON_DPAD_UP:
                return C_dpad_up;

            case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
                return C_dpad_right;

            case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
                return C_dpad_down;

            case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
                return C_dpad_left;

            default:
                return K_unknown;
        }
    }
    int Input::keytype_to_glfw_gamepad_button(KeyType key) {
        switch (key) {
            case C_a:
                return GLFW_GAMEPAD_BUTTON_A;

            case C_b:
                return GLFW_GAMEPAD_BUTTON_B;

            case C_x:
                return GLFW_GAMEPAD_BUTTON_X;

            case C_y:
                return GLFW_GAMEPAD_BUTTON_Y;

            case C_left_bumper:
                return GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;

            case C_right_bumper:
                return GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;

            case C_back:
                return GLFW_GAMEPAD_BUTTON_BACK;

            case C_start:
                return GLFW_GAMEPAD_BUTTON_START;

            case C_guide:
                return GLFW_GAMEPAD_BUTTON_GUIDE;

            case C_left_thumb:
                return GLFW_GAMEPAD_BUTTON_LEFT_THUMB;

            case C_right_thumb:
                return GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;

            case C_dpad_up:
                return GLFW_GAMEPAD_BUTTON_DPAD_UP;

            case C_dpad_right:
                return GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;

            case C_dpad_down:
                return GLFW_GAMEPAD_BUTTON_DPAD_DOWN;

            case C_dpad_left:
                return GLFW_GAMEPAD_BUTTON_DPAD_LEFT;

            case C_left_x:
                return GLFW_GAMEPAD_AXIS_LEFT_X;
            
            case C_left_y:
                return GLFW_GAMEPAD_AXIS_LEFT_Y;

            case C_right_x:
                return GLFW_GAMEPAD_AXIS_RIGHT_X;

            case C_right_y:
                return GLFW_GAMEPAD_AXIS_RIGHT_Y;

            case C_left_trigger:
                return GLFW_GAMEPAD_AXIS_LEFT_TRIGGER;

            case C_right_trigger:
                return GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;

            default:
                return -1;
        }
    }
    int Input::keytype_to_glfw_joystick(KeyType key) {
        switch (key) {
            case C_JOYSTICK_1:  return GLFW_JOYSTICK_1;
            case C_JOYSTICK_2:  return GLFW_JOYSTICK_2;
            case C_JOYSTICK_3:  return GLFW_JOYSTICK_3;
            case C_JOYSTICK_4:  return GLFW_JOYSTICK_4;
            case C_JOYSTICK_5:  return GLFW_JOYSTICK_5;
            case C_JOYSTICK_6:  return GLFW_JOYSTICK_6;
            case C_JOYSTICK_7:  return GLFW_JOYSTICK_7;
            case C_JOYSTICK_8:  return GLFW_JOYSTICK_8;
            case C_JOYSTICK_9:  return GLFW_JOYSTICK_9;
            case C_JOYSTICK_10: return GLFW_JOYSTICK_10;
            case C_JOYSTICK_11: return GLFW_JOYSTICK_11;
            case C_JOYSTICK_12: return GLFW_JOYSTICK_12;
            case C_JOYSTICK_13: return GLFW_JOYSTICK_13;
            case C_JOYSTICK_14: return GLFW_JOYSTICK_14;
            case C_JOYSTICK_15: return GLFW_JOYSTICK_15;
            case C_JOYSTICK_16: return GLFW_JOYSTICK_16;

            default:
                return -1;
        }
    }

    bool Input::is_pressed(KeyType key) {
        return this->keys[key];
    }

    KeyType Input::glfw_to_keytype(int key) {
        switch (key) {
            // Letters
            case GLFW_KEY_A: return K_a;
            case GLFW_KEY_B: return K_b;
            case GLFW_KEY_C: return K_c;
            case GLFW_KEY_D: return K_d;
            case GLFW_KEY_E: return K_e;
            case GLFW_KEY_F: return K_f;
            case GLFW_KEY_G: return K_g;
            case GLFW_KEY_H: return K_h;
            case GLFW_KEY_I: return K_i;
            case GLFW_KEY_J: return K_j;
            case GLFW_KEY_K: return K_k;
            case GLFW_KEY_L: return K_l;
            case GLFW_KEY_M: return K_m;
            case GLFW_KEY_N: return K_n;
            case GLFW_KEY_O: return K_o;
            case GLFW_KEY_P: return K_p;
            case GLFW_KEY_Q: return K_q;
            case GLFW_KEY_R: return K_r;
            case GLFW_KEY_S: return K_s;
            case GLFW_KEY_T: return K_t;
            case GLFW_KEY_U: return K_u;
            case GLFW_KEY_V: return K_v;
            case GLFW_KEY_W: return K_w;
            case GLFW_KEY_X: return K_x;
            case GLFW_KEY_Y: return K_y;
            case GLFW_KEY_Z: return K_z;

            // Numbers
            case GLFW_KEY_0: return K_0;
            case GLFW_KEY_1: return K_1;
            case GLFW_KEY_2: return K_2;
            case GLFW_KEY_3: return K_3;
            case GLFW_KEY_4: return K_4;
            case GLFW_KEY_5: return K_5;
            case GLFW_KEY_6: return K_6;
            case GLFW_KEY_7: return K_7;
            case GLFW_KEY_8: return K_8;
            case GLFW_KEY_9: return K_9;

            // Function keys
            case GLFW_KEY_F1: return K_f1;
            case GLFW_KEY_F2: return K_f2;
            case GLFW_KEY_F3: return K_f3;
            case GLFW_KEY_F4: return K_f4;
            case GLFW_KEY_F5: return K_f5;
            case GLFW_KEY_F6: return K_f6;
            case GLFW_KEY_F7: return K_f7;
            case GLFW_KEY_F8: return K_f8;
            case GLFW_KEY_F9: return K_f9;
            case GLFW_KEY_F10: return K_f10;
            case GLFW_KEY_F11: return K_f11;
            case GLFW_KEY_F12: return K_f12;

            // Arrow keys
            case GLFW_KEY_UP: return K_up;
            case GLFW_KEY_DOWN: return K_down;
            case GLFW_KEY_LEFT: return K_left;
            case GLFW_KEY_RIGHT: return K_right;

            // Navigation
            case GLFW_KEY_INSERT: return K_insert;
            case GLFW_KEY_DELETE: return K_delete;
            case GLFW_KEY_HOME: return K_home;
            case GLFW_KEY_END: return K_end;
            case GLFW_KEY_PAGE_UP: return K_page_up;
            case GLFW_KEY_PAGE_DOWN: return K_page_down;

            // Modifiers
            case GLFW_KEY_LEFT_SHIFT: return K_left_shift;
            case GLFW_KEY_RIGHT_SHIFT: return K_right_shift;
            case GLFW_KEY_LEFT_CONTROL: return K_left_control;
            case GLFW_KEY_RIGHT_CONTROL: return K_right_control;
            case GLFW_KEY_LEFT_ALT: return K_left_alt;
            case GLFW_KEY_RIGHT_ALT: return K_right_alt;
            case GLFW_KEY_LEFT_SUPER: return K_left_super;
            case GLFW_KEY_RIGHT_SUPER: return K_right_super;

            // Whitespace
            case GLFW_KEY_SPACE: return K_space;
            case GLFW_KEY_TAB: return K_tab;
            case GLFW_KEY_ENTER: return K_enter;
            case GLFW_KEY_BACKSPACE: return K_backspace;
            case GLFW_KEY_ESCAPE: return K_escape;

            // Symbols
            case GLFW_KEY_APOSTROPHE: return K_apostrophe;
            case GLFW_KEY_COMMA: return K_comma;
            case GLFW_KEY_MINUS: return K_minus;
            case GLFW_KEY_PERIOD: return K_period;
            case GLFW_KEY_SLASH: return K_slash;
            case GLFW_KEY_SEMICOLON: return K_semicolon;
            case GLFW_KEY_EQUAL: return K_equal;
            case GLFW_KEY_LEFT_BRACKET: return K_left_bracket;
            case GLFW_KEY_RIGHT_BRACKET: return K_right_bracket;
            case GLFW_KEY_BACKSLASH: return K_backslash;
            case GLFW_KEY_GRAVE_ACCENT: return K_grave_accent;

            // Lock keys
            case GLFW_KEY_CAPS_LOCK: return K_caps_lock;
            case GLFW_KEY_SCROLL_LOCK: return K_scroll_lock;
            case GLFW_KEY_NUM_LOCK: return K_num_lock;

            // Numpad
            case GLFW_KEY_KP_0: return K_num_0;
            case GLFW_KEY_KP_1: return K_num_1;
            case GLFW_KEY_KP_2: return K_num_2;
            case GLFW_KEY_KP_3: return K_num_3;
            case GLFW_KEY_KP_4: return K_num_4;
            case GLFW_KEY_KP_5: return K_num_5;
            case GLFW_KEY_KP_6: return K_num_6;
            case GLFW_KEY_KP_7: return K_num_7;
            case GLFW_KEY_KP_8: return K_num_8;
            case GLFW_KEY_KP_9: return K_num_9;

            case GLFW_KEY_KP_DECIMAL: return K_num_decimal;
            case GLFW_KEY_KP_DIVIDE: return K_num_divide;
            case GLFW_KEY_KP_MULTIPLY: return K_num_multiply;
            case GLFW_KEY_KP_SUBTRACT: return K_num_subtract;
            case GLFW_KEY_KP_ADD: return K_num_add;
            case GLFW_KEY_KP_ENTER: return K_num_enter;
            case GLFW_KEY_KP_EQUAL: return K_num_equal;

            // Misc
            case GLFW_KEY_PRINT_SCREEN: return K_print_screen;
            case GLFW_KEY_PAUSE: return K_pause;

            // Mouse
            case GLFW_MOUSE_BUTTON_LEFT: return M_lmb;
            case GLFW_MOUSE_BUTTON_MIDDLE: return M_mmb;
            case GLFW_MOUSE_BUTTON_RIGHT: return M_rmb;
            case GLFW_MOUSE_BUTTON_4: return M_4;
            case GLFW_MOUSE_BUTTON_5: return M_5;
            case GLFW_MOUSE_BUTTON_6: return M_6;
            case GLFW_MOUSE_BUTTON_7: return M_7;
            case GLFW_MOUSE_BUTTON_8: return M_8;

            default:
                return K_unknown;
        }
    }

    int Input::keytype_to_glfw_mouse(int button) {
        switch (button) {
            case KeyType::M_lmb: return GLFW_MOUSE_BUTTON_LEFT;
            case KeyType::M_mmb: return GLFW_MOUSE_BUTTON_MIDDLE;
            case KeyType::M_rmb: return GLFW_MOUSE_BUTTON_RIGHT;

            case KeyType::M_4: return GLFW_MOUSE_BUTTON_4;
            case KeyType::M_5: return GLFW_MOUSE_BUTTON_5;
            case KeyType::M_6: return GLFW_MOUSE_BUTTON_6;
            case KeyType::M_7: return GLFW_MOUSE_BUTTON_7;
            case KeyType::M_8: return GLFW_MOUSE_BUTTON_8;
            default: return -1;
        }
    }
};
