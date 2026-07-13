#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <bits/stdc++.h>
#include <algorithm>

#define VIVIANITE_VSYNC_TRUE 1
#define VIVIANITE_VSYNC_FALSE 0
#define VIVIANITE_VSYNC_HALF 2

namespace vivianite {
    struct shader {
        std::string frag_path, vert_path;
        std::string frag_raw, vert_raw;
        GLuint frag, vert, program;
    };

    struct mesh {
        std::vector<float> vertices;
        GLuint vao;
        size_t vertex_count;
    };

    struct model {
        mesh obj;
        glm::vec3 position;
    };

    class renderer {
        public:
            int width = 800;
            int height = 600;
            
            int vsync = VIVIANITE_VSYNC_FALSE;

            const char* title = "Vivianite Window";

            float FOV = 90;

            bool init_status = true;

            GLFWwindow* window;
            shader program;

            int gl_major_version = 4;
            int gl_minor_version = 6;

            void (*update_func)(vivianite::renderer*);
            void (*setup_func)(vivianite::renderer*);
            void (*exit_func)(vivianite::renderer*);

            std::vector<model> render_queue = {};
            std::vector<int> keys = {};
            std::vector<int> scancodes = {};

            glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 5.0f);
            glm::mat4 projection;

            double delta_time = 0.0;
            double time = 0.0;

            static void error_callback(int error, const char* description) {
                fprintf(stderr, "Error: %s\n", description);
            }

            renderer() {
                if (!glfwInit()) {
                    this->init_status = false;
                    return;
                }
                glfwSetErrorCallback(this->error_callback);

                this->projection = glm::perspective(
                    glm::radians(this->FOV),
                    (float)this->width / (float)this->height,
                    0.1f,
                    100.0f
                );
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

                // Compile fragment shader
                glCompileShader(this->program.frag);

                // Check if it compiled correctly
                GLint isCompiled = 0;
                char infoLog[512];
                glGetShaderiv(this->program.frag, GL_COMPILE_STATUS, &isCompiled);
                if(isCompiled == GL_FALSE) {
                    glGetShaderInfoLog(this->program.frag, 512, nullptr, infoLog);
                    std::cout << "Fragment shader error:\n" << infoLog << "\n";

                    glDeleteShader(this->program.frag); // Don't leak the shader.
                    return;
                }

                // Compile vertex shader
                glCompileShader(this->program.vert);

                // Check if it compiled correctly
                isCompiled = 0;
                glGetShaderiv(this->program.vert, GL_COMPILE_STATUS, &isCompiled);
                if(isCompiled == GL_FALSE) {
                    glGetShaderInfoLog(this->program.vert, 512, nullptr, infoLog);
                    std::cout << "Vertex shader error:\n" << infoLog << "\n";

                    glDeleteShader(this->program.frag); // Don't leak the shader.
                    return;
                }

                this->program.program = glCreateProgram();

                glAttachShader(this->program.program, this->program.vert);
                glAttachShader(this->program.program, this->program.frag);

                glLinkProgram(this->program.program);
            }

            GLuint upload_mesh(const std::vector<float> &mesh) {
                GLuint vao;
                GLuint vbo;

                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vbo);

                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);

                glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(float), mesh.data(), GL_STATIC_DRAW);

                // Position
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                
                // Color
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                return vao;
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

                glEnable(GL_DEPTH_TEST);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glDisable(GL_CULL_FACE);

                glfwGetFramebufferSize(window, &width, &height);
                glViewport(0, 0, width, height);

                glfwSwapInterval(this->vsync);

                return true;
            }

            void apply_settings() {
                glfwSetWindowTitle(window, this->title);
                glfwSetWindowSize(window, this->width, this->height);
                glfwSwapInterval(this->vsync);

                glUniformMatrix4fv(
                    glGetUniformLocation(this->program.program, "projection"),
                    1,
                    GL_FALSE,
                    glm::value_ptr(this->projection)
                );
            }

            void run() {
                this->setup_func(this);

                glUseProgram(this->program.program);

                glUniformMatrix4fv(
                    glGetUniformLocation(this->program.program, "projection"),
                    1,
                    GL_FALSE,
                    glm::value_ptr(this->projection)
                );

                double last = glfwGetTime();

                while (!glfwWindowShouldClose(window)) {
                    glUseProgram(this->program.program);

                    this->time = glfwGetTime();
                    this->delta_time = this->time - last;
                    last = this->time;

                    this->update_func(this);

                    glm::mat4 view = glm::translate(
                        glm::mat4(1.0f),
                        -this->camera_pos
                    );

                    glUniformMatrix4fv(
                        glGetUniformLocation(this->program.program, "view"),
                        1,
                        GL_FALSE,
                        glm::value_ptr(view)
                    );

                    for (model obj : this->render_queue) {
                        glBindVertexArray(obj.obj.vao);

                        glm::mat4 modelMat = glm::translate(
                            glm::mat4(1.0f),
                            obj.position
                        );

                        glUniformMatrix4fv(
                            glGetUniformLocation(this->program.program, "model"),
                            1,
                            GL_FALSE,
                            glm::value_ptr(modelMat)
                        );

                        glDrawArrays(GL_TRIANGLES, 0, obj.obj.vertex_count);
                    }

                    glfwPollEvents();
                    glfwSwapBuffers(window);
                }
            }

            void exit() {
                this->exit_func(this);
                glfwSetWindowShouldClose(this->window, GLFW_TRUE);
            }

            ~renderer() {
                this->exit_func(this);

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

        private:
            void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
                if ((std::count(this->keys.begin(), this->keys.end(), key) == 0) && (action == GLFW_PRESS)) {
                    this->keys.push_back(key);
                    this->scancodes.push_back(scancode);
                }
                else if ((std::count(this->keys.begin(), this->keys.end(), key) > 0) && (action == GLFW_RELEASE)) {
                    this->keys.erase(std::find(this->keys.begin(), this->keys.end(), key));
                    this->scancodes.erase(std::find(this->scancodes.begin(), this->scancodes.end(), scancode));
                }
            }
    };
};
