#include "include.hpp"

namespace vivianite {
    Input::Input() {
        auto* r_ctx = (renderer*)r_ptr;

        glfwSetKeyCallback(r_ctx->window, this->key_callback);
    }

    void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        vivianite::renderer* r_ctx = (renderer*)glfwGetWindowUserPointer(window);

        Input* i_ctx = r_ctx->i_ctx;

        for (auto* callback: i_ctx->key_callbacks) {
            callback(i_ctx->e_ptr, i_ctx->r_ptr);
        }
    }
};
