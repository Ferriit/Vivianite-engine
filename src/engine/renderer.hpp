#pragma once

#include "logging.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <string>
#include <cstdio>
#include <bits/stdc++.h>

#define VIVIANITE_VSYNC_TRUE 1
#define VIVIANITE_VSYNC_FALSE 0
#define VIVIANITE_VSYNC_HALF 2

namespace vivianite {
    struct shader {
        std::string frag_path, vert_path;
        std::string frag_raw, vert_raw;
        GLuint frag, vert, program;
    };

    // TODO: Add a material format
    struct alignas(16) material {
        glm::vec3 albedo;
        float shininess;
        float specular_strength;
    };

    struct alignas(16) mesh {
        std::vector<float> vertices;
        GLuint vao;
        size_t vertex_count;
    };

    struct alignas(16) model {
        mesh obj;
        glm::vec3 position;
        glm::vec3 rotation;
        material mat;
    };

    struct alignas(16) light {
        glm::vec4 position;
        glm::vec4 color;
        float radius, strength;
        float linear;
        float quadratic;
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

            void (*update_func)(vivianite::renderer*, void*);
            void (*setup_func)(vivianite::renderer*, void*);
            void (*exit_func)(vivianite::renderer*, void*);

            void* engine_ctx;

            std::vector<model> render_queue = {};

            glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 5.0f);
            glm::mat4 projection;

            std::vector<light> lights = {};
            GLuint light_ssbo;

            double delta_time = 0.0;
            double time = 0.0;

            Logging logger;

            static void error_callback(int error, const char* description) {
                fprintf(stderr, "Error: %s\n", description);
            }

            renderer() {
                if (!glfwInit()) {
                    this->init_status = false;
                    logger.log(logger.FATAL, "Failed to initialize GLFW");
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
                logger.log(logger.INFO, "Reading GLSL shaders");

                // Frag
                std::ifstream frag_file(this->program.frag_path);

                if (!frag_file.is_open()) {
                    logger.log(logger.FATAL, "Failed to read Fragment shader");
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
                    logger.log(logger.FATAL, "Failed to read Vertex shader");
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
                logger.log(logger.INFO, "Creating shader program");

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
                    logger.log(logger.ERROR, "Fragment shader error:\n%512s", infoLog);

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
                    logger.log(logger.ERROR, "Vertex shader error:\n%512s", infoLog);

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
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                
                // Color
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);

                // Normal
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
                glEnableVertexAttribArray(2);

                return vao;
            }

            void init_SSBOs() {
                logger.log(logger.INFO, "Uploading SSBOs");
                glGenBuffers(1, &this->light_ssbo);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->light_ssbo);

                glBufferData(
                    GL_SHADER_STORAGE_BUFFER,
                    lights.size() * sizeof(light),
                    lights.data(),
                    GL_DYNAMIC_DRAW
                );

                glBindBufferBase(
                    GL_SHADER_STORAGE_BUFFER,
                    0,
                    light_ssbo
                );

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }

            void upload_lights() {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->light_ssbo);

                glBufferData(
                    GL_SHADER_STORAGE_BUFFER,
                    this->lights.size() * sizeof(light),
                    this->lights.data(),
                    GL_DYNAMIC_DRAW
                );

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }

            bool initialize() {
                logger.log(logger.INFO, "Initializing OpenGL and GLAD");
                // OpenGL 4.6 CORE
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->gl_major_version);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, this->gl_minor_version);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

                // GLFW window
                this->window = glfwCreateWindow(this->width, this->height, this->title, NULL, NULL);

                if (!this->window) {
                    logger.log(logger.FATAL, "Failed to open Window");
                    return false;
                }

                glfwMakeContextCurrent(window);
                glfwSetWindowUserPointer(window, this);
                
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
                logger.log(logger.NOTICE, "Applying GLFW settings");

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
                logger.log(logger.INFO, "Starting main setup");
                this->setup_func(this, engine_ctx);

                logger.log(logger.INFO, "Assigning shader program");
                glUseProgram(this->program.program);

                glUniformMatrix4fv(
                    glGetUniformLocation(this->program.program, "projection"),
                    1,
                    GL_FALSE,
                    glm::value_ptr(this->projection)
                );

                double last = glfwGetTime();

                logger.log(logger.INFO, "Starting main update loop");

                while (!glfwWindowShouldClose(window)) {
                    glUseProgram(this->program.program);

                    this->time = glfwGetTime();
                    this->delta_time = this->time - last;
                    last = this->time;

                    this->update_func(this, engine_ctx);

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

                    // Camera
                    glUniform3fv(
                        glGetUniformLocation(this->program.program, "camera_pos"),
                        1,
                        glm::value_ptr(this->camera_pos)
                    );

                    // Lights
                    glUniform1i(
                        glGetUniformLocation(this->program.program, "light_count"),
                        static_cast<int>(this->lights.size())
                    );

                    // Per-Model stuff
                    for (model obj : this->render_queue) {
                        glBindVertexArray(obj.obj.vao);

                        // Translation
                        glm::mat4 modelMat = glm::translate(
                            glm::mat4(1.0f),
                            obj.position
                        );
                        
                        // Rotation (X, Y, Z)
                        modelMat = glm::rotate(
                            modelMat,
                            obj.rotation.x,
                            glm::vec3(1.0f, 0.0f, 0.0f)
                        );

                        modelMat = glm::rotate(
                            modelMat,
                            obj.rotation.y,
                            glm::vec3(0.0f, 1.0f, 0.0f)
                        );
                        
                        modelMat = glm::rotate(
                            modelMat,
                            obj.rotation.z,
                            glm::vec3(0.0f, 0.0f, 1.0f)
                        );

                        // Upload model
                        glUniformMatrix4fv(
                            glGetUniformLocation(this->program.program, "model"),
                            1,
                            GL_FALSE,
                            glm::value_ptr(modelMat)
                        );

                        // Material
                        glUniform3fv(
                            glGetUniformLocation(this->program.program, "material.albedo"),
                            1,
                            glm::value_ptr(obj.mat.albedo)
                        );

                        glUniform1f(
                            glGetUniformLocation(this->program.program, "material.shininess"),
                            obj.mat.shininess
                        );

                        glUniform1f(
                            glGetUniformLocation(this->program.program, "material.specularStrength"),
                            obj.mat.specular_strength
                        );

                        glDrawArrays(GL_TRIANGLES, 0, obj.obj.vertex_count);
                    }

                    glfwPollEvents();
                    glfwSwapBuffers(window);
                }
            }

            void exit() {
                this->exit_func(this, engine_ctx);
                glfwSetWindowShouldClose(this->window, GLFW_TRUE);
            }

            ~renderer() {
                logger.log(logger.NOTICE, "Exitting... Check log for errors.");
                this->exit_func(this, engine_ctx);

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
    };
};
