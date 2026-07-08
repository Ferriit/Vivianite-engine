#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#define VIVIANITE_VSYNC_TRUE 1
#define VIVIANITE_VSYNC_FALSE 0
#define VIVIANITE_VSYNC_HALF 2

namespace vivianite {
    class renderer {
        public:
            int width = 800;
            int height = 600;
            
            int vsync = VIVIANITE_VSYNC_FALSE;

            const char* title = "Vivianite Window";

            int FOV = 90;

            bool init_status = true;

            GLFWwindow* window;

            int gl_major_version = 4;
            int gl_minor_version = 6;

            void (*update_func)(vivianite::renderer*);
            void (*setup_func)(vivianite::renderer*);

            double delta_time;

            static void error_callback(int error, const char* description) {
                fprintf(stderr, "Error: %s\n", description);
            }

            renderer() {
                // GLFW
                if (!glfwInit()) {
                    this->init_status = false;
                    return;
                }
                glfwSetErrorCallback(this->error_callback);
            }

            bool initialize() {
                // OpenGL 4.6 CORE
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->gl_major_version);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, this->gl_minor_version);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

                // GLFW window
                this->window = glfwCreateWindow(this->width, this->height, this->title, NULL, NULL);

                if (!this->window) {
                    return false;
                }

                glfwMakeContextCurrent(window);
                
                // GLAD init
                gladLoadGL(glfwGetProcAddress);

                glfwSwapInterval(this->vsync);

                return true;
            }

            void run() {
                this->setup_func(this);

                double last = glfwGetTime();

                while (!glfwWindowShouldClose(window)) {
                    double now = glfwGetTime();
                    this->delta_time = now - last;
                    last = now;

                    update_func(this);

                    glfwPollEvents();
                    glfwSwapBuffers(window);
                }
            }

            void exit() {
                glfwSetWindowShouldClose(this->window, GLFW_TRUE);
            }

            ~renderer() {
                if (window) {
                    glfwDestroyWindow(window);
                }
                glfwTerminate();
            }

            bool check_status() {
                return this->init_status;
            }
    };
};
