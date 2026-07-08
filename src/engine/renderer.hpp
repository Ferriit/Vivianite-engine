#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

#define VIVIANITE_VSYNC_TRUE 1
#define VIVIANITE_VSYNC_FALSE 0
#define VIVIANITE_VSYNC_HALF 2

namespace vivianite {
    struct shader {
        std::string frag_path, vert_path;
        std::string frag_raw, vert_raw;
        GLuint frag, vert, program;
    };

    class renderer {
        public:
            int width = 800;
            int height = 600;
            
            int vsync = VIVIANITE_VSYNC_FALSE;

            const char* title = "Vivianite Window";

            int FOV = 90;

            bool init_status = true;

            GLFWwindow* window;
            shader program;

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

            bool read_shaders() {
                // Frag
                std::ifstream frag_file(this->program.frag_path);

                if (!frag_file.is_open()) {
                    return false;
                }

                this->program.frag_raw = std::string(
                    (std::istreambuf_iterator<char>(frag_file)),
                    std::istreambuf_iterator<char>()
                );

                frag_file.close();

                // Vert
                std::ifstream vert_file(this->program.vert_path);

                if (!vert_file.is_open()) {
                    return false;
                }

                this->program.vert_raw = std::string(
                    (std::istreambuf_iterator<char>(vert_file)),
                    std::istreambuf_iterator<char>()
                );

                vert_file.close();

                return true;
            }

            void create_shaders() {
                read_shaders(); // Populates frag_raw and vert_raw

                this->program.frag = glCreateShader(GL_FRAGMENT_SHADER);
                this->program.vert = glCreateShader(GL_VERTEX_SHADER);

                const char* frag_src = this->program.frag_raw.c_str();
                const char* vert_src = this->program.vert_raw.c_str();

                // TODO: Add proper error checking

                glShaderSource(this->program.frag, 1, &frag_src, nullptr);
                glShaderSource(this->program.vert, 1, &vert_src, nullptr);

                glCompileShader(this->program.frag);
                glCompileShader(this->program.vert);

                this->program.program = glCreateProgram();

                glAttachShader(this->program.program, this->program.vert);
                glAttachShader(this->program.program, this->program.frag);

                glLinkProgram(this->program.program);
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

                glfwGetFramebufferSize(window, &width, &height);
                glViewport(0, 0, width, height);

                glUseProgram(this->program.program);

                glfwSwapInterval(this->vsync);

                return true;
            }

            void apply_settings() {
                glfwSetWindowTitle(window, this->title);
                glfwSetWindowSize(window, this->width, this->height);
                glfwSwapInterval(this->vsync);
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
                glDeleteShader(this->program.frag);
                glDeleteShader(this->program.vert);

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
