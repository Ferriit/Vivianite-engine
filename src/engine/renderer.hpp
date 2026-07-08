#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <GL/glext.h>

#include <GLFW/glfw3.h>

#include <iostream>

namespace vivianite {
    class renderer {
        public:
            int width = 800;
            int height = 600;
            
            char* title = "Vivianite Window";

            int FOV = 90;

            bool init_status = true;

            GLFWwindow* window;

            int gl_major_version = 4;
            int gl_minor_version = 6;

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
                
                // GLEW init

                return true;
            }

            ~renderer() {
                glfwDestroyWindow(window);
                glfwTerminate();
            }

            bool check_status() {
                return this->init_status;
            }
    };
};
