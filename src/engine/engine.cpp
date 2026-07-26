#include "renderer.hpp"
#include "logging.hpp"
#include "scheduler.hpp"

#include <GL/gl.h>
#include <glad/gl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace vivianite {
    mesh load_obj(const char* filename) {
        Logging().log(Logging::INFO, "Reading file \"%s\"", filename);

        std::ifstream file(filename);

        if (!file.is_open()) {
            Logging().log(Logging::ERROR, "Unable to open model \"%s\"", filename);
            return {};
        }

        std::vector<float> x_list;
        std::vector<float> y_list;
        std::vector<float> z_list;

        std::vector<float> nx_list;
        std::vector<float> ny_list;
        std::vector<float> nz_list;

        struct Face {
            std::array<int, 3> vertex;
            std::array<int, 3> normal;
        };

        std::vector<Face> faces;

        std::string line;

        while (std::getline(file, line)) {

            if (line.rfind("v ", 0) == 0) {
                std::stringstream ss(line.substr(2));

                float x, y, z;
                ss >> x >> y >> z;

                x_list.push_back(x);
                y_list.push_back(y);
                z_list.push_back(z);
            }

            else if (line.rfind("vn ", 0) == 0) {
                std::stringstream ss(line.substr(3));

                float nx, ny, nz;
                ss >> nx >> ny >> nz;

                nx_list.push_back(nx);
                ny_list.push_back(ny);
                nz_list.push_back(nz);
            }

            else if (line.rfind("f ", 0) == 0) {
                std::stringstream ss(line.substr(2));

                Face face;

                for (int i = 0; i < 3; i++) {
                    std::string token;
                    ss >> token;

                    size_t s1 = token.find('/');
                    size_t s2 = token.find('/', s1 + 1);

                    face.vertex[i] = std::stoi(token.substr(0, s1)) - 1;
                    face.normal[i] = std::stoi(token.substr(s2 + 1)) - 1;
                }

                faces.push_back(face);
            }
        }

        if (x_list.empty())
            return {};

        float min_x = x_list[0], max_x = x_list[0];
        float min_y = y_list[0], max_y = y_list[0];
        float min_z = z_list[0], max_z = z_list[0];

        for (size_t i = 1; i < x_list.size(); i++) {
            min_x = std::min(min_x, x_list[i]);
            max_x = std::max(max_x, x_list[i]);

            min_y = std::min(min_y, y_list[i]);
            max_y = std::max(max_y, y_list[i]);

            min_z = std::min(min_z, z_list[i]);
            max_z = std::max(max_z, z_list[i]);
        }

        float cx = (min_x + max_x) * 0.5f;
        float cy = (min_y + max_y) * 0.5f;
        float cz = (min_z + max_z) * 0.5f;

        float dx = (max_x - min_x == 0) ? 1.0f : max_x - min_x;
        float dy = (max_y - min_y == 0) ? 1.0f : max_y - min_y;
        float dz = (max_z - min_z == 0) ? 1.0f : max_z - min_z;

        std::vector<float> vertices;
        vertices.reserve(faces.size() * 3 * 9);

        for (const Face& face : faces) {
            for (int i = 0; i < 3; i++) {

                int vi = face.vertex[i];
                int ni = face.normal[i];

                float x = x_list[vi];
                float y = y_list[vi];
                float z = z_list[vi];

                float r = (x - min_x) / dx;
                float g = (y - min_y) / dy;
                float b = (z - min_z) / dz;

                x -= cx;
                y -= cy;
                z -= cz;

                // Position
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Color
                vertices.push_back(r);
                vertices.push_back(g);
                vertices.push_back(b);

                // Normal
                vertices.push_back(nx_list[ni]);
                vertices.push_back(ny_list[ni]);
                vertices.push_back(nz_list[ni]);
            }
        }

        return mesh{
            std::move(vertices),
            0,
            faces.size() * 3
        };
    }

    class engine {
        public:
            int status = 0;

            std::array<bool, GLFW_KEY_LAST + 1> keys = {};
            std::unordered_set<int> scancodes = {};
            
            vivianite::renderer r_ctx;

            engine() {
                if (!r_ctx.check_status()) {
                    status = 1;
                    return;
                }

                r_ctx.initialize();

                r_ctx.program.frag_path = "assets/frag.glsl";
                r_ctx.program.vert_path = "assets/vert.glsl";

                r_ctx.create_shaders();

                Logging().log(Logging::INFO, "Assigning functions");
                r_ctx.setup_func = this->setup;
                r_ctx.update_func = this->update;
                r_ctx.exit_func = this->exit;

                r_ctx.engine_ctx = this;

                r_ctx.run();
            }

            static void setup(vivianite::renderer* r_ctx, void* ctx) {
                auto* e_ctx = (vivianite::engine*)ctx;

                // Set up cube
                vivianite::mesh cube_obj = vivianite::load_obj("assets/cube.obj");
                GLuint cube = r_ctx->upload_mesh(cube_obj.vertices);

                cube_obj.vao = cube;

                vivianite::model cube_model = {.obj=cube_obj, .position=glm::vec3(0.0f, 0.0f, 0.0f)};

                cube_model.mat = (vivianite::material){
                    .albedo = glm::vec3(1.0f, 1.0f, 1.0f),
                    .shininess = 32.0f,
                    .specular_strength = 0.5f
                };

                Logging().log(Logging::INFO, "Uploading models");
                r_ctx->render_queue.push_back(cube_model);

                r_ctx->vsync = VIVIANITE_VSYNC_TRUE;
                r_ctx->apply_settings();

                Logging().log(Logging::INFO, "Uploading lights");
                // Set up lights
                r_ctx->lights.push_back(
                    (vivianite::light){
                        .position=glm::vec4(-5.0f, 0.0f, 0.0f, 1.0f),
                        .color=glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
                        .radius=50.0f,
                        .strength=1.0f,
                        .linear=0.09f,
                        .quadratic=0.032f
                    }
                );
                r_ctx->lights.push_back(
                    (vivianite::light){
                        .position=glm::vec4(5.0f, 0.0f, 0.0f, 1.0f),
                        .color=glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
                        .radius=50.0f,
                        .strength=1.0f,
                        .linear=0.09f,
                        .quadratic=0.032f
                    }
                );
                r_ctx->init_SSBOs();

                glfwSetKeyCallback(r_ctx->window, e_ctx->key_callback);
                Logging().log(Logging::log_level::NOTICE, "ENGINE SETUP DONE");
            }

            static void update(vivianite::renderer* r_ctx, void* ctx) {
                auto* e_ctx = (vivianite::engine*)ctx;

                glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Rotate
                r_ctx->render_queue[0].rotation.y += 0.03f;  
                r_ctx->render_queue[0].rotation.x += 0.03f;
                r_ctx->render_queue[0].rotation.z += 0.03f;

                if (e_ctx->keys[GLFW_KEY_ESCAPE] == true) {
                    r_ctx->exit();
                }
            }

            static void exit(vivianite::renderer* r_ctx, void* ctx) {
                auto* e_ctx = (vivianite::engine*)ctx;

                Logging().log(Logging::log_level::INFO, "%f FPS (%f ms)", 1 / r_ctx->delta_time, r_ctx->delta_time * 1000.0f);
            }

        private:
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
                auto* r_ctx = static_cast<vivianite::renderer*>(glfwGetWindowUserPointer(window));
                if (!r_ctx)
                    return;

                auto* e_ctx = static_cast<vivianite::engine*>(r_ctx->engine_ctx);
                if (!e_ctx)
                    return;

                if (action == GLFW_PRESS) {
                    e_ctx->keys[key] = true;
                    e_ctx->scancodes.insert(scancode);
                }
                else if (action == GLFW_RELEASE) {
                    e_ctx->keys[key] = false;
                    e_ctx->scancodes.erase(scancode);
                }
            }
    };
};


int main() {
    vivianite::engine ctx;

    return ctx.status;
}
