#include "include.hpp"

namespace vivianite {
    void Input::initialize() {
        auto* r_ctx = (renderer*)r_ptr;

        keys.resize(KeyType::K_unknown);

        glfwSetKeyCallback(r_ctx->window, key_callback);
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

        i_ctx->keys[i_ctx->glfw_to_keytype(key)] = action == GLFW_PRESS;

        for (auto& callback : i_ctx->key_callbacks) {
            callback(event);
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

            default:
                return K_unknown;
        }
    }
};
