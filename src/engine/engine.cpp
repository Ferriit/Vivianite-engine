#include "renderer.hpp"
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
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Unable to open model file.\n";
            return {};
        }

        std::vector<float> x_list;
        std::vector<float> y_list;
        std::vector<float> z_list;

        std::vector<std::array<int, 3>> faces;

        std::string line;

        // Read vertices and faces
        while (std::getline(file, line)) {

            if (line.rfind("v ", 0) == 0) {
                std::stringstream ss(line.substr(2));

                float x, y, z;
                ss >> x >> y >> z;

                x_list.push_back(x);
                y_list.push_back(y);
                z_list.push_back(z);
            }

            else if (line.rfind("f ", 0) == 0) {
                std::stringstream ss(line.substr(2));

                std::array<int, 3> face;

                for (int i = 0; i < 3; i++) {
                    std::string vertex;
                    ss >> vertex;

                    // Extract the vertex index before the first '/'
                    size_t slash = vertex.find('/');

                    if (slash != std::string::npos) {
                        vertex = vertex.substr(0, slash);
                    }

                    face[i] = std::stoi(vertex) - 1;
                }

                faces.push_back(face);
            }
        }


        if (x_list.empty())
            return {};


        // Find bounds for coloring
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


        // Expand triangles into a VAO-friendly vertex array
        std::vector<float> vertices;
        vertices.reserve(faces.size() * 3 * 6);

        for (auto& face : faces) {
            for (int index : face) {

                float x = x_list[index];
                float y = y_list[index];
                float z = z_list[index];

                float r = (x - min_x) / dx;
                float g = (y - min_y) / dy;
                float b = (z - min_z) / dz;

                // Center model
                x -= cx;
                y -= cy;
                z -= cz;

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                vertices.push_back(r);
                vertices.push_back(g);
                vertices.push_back(b);
            }
        }


        size_t vertex_count = vertices.size() / 6;

        return mesh{
            std::move(vertices),
            0,
            vertex_count
        };
    }

    class engine {
        public:
            int status = 0;

            std::array<bool, GLFW_KEY_LAST + 1> keys = {};
            std::unordered_set<int> scancodes = {};

            engine() {
                vivianite::renderer r_ctx;

                if (!r_ctx.check_status()) {
                    status = 1;
                    return;
                }

                r_ctx.initialize();

                r_ctx.program.frag_path = "assets/frag.glsl";
                r_ctx.program.vert_path = "assets/vert.glsl";

                r_ctx.create_shaders();
                
                r_ctx.setup_func = this->setup;
                r_ctx.update_func = this->update;
                r_ctx.exit_func = this->exit;

                r_ctx.engine_ctx = this;

                r_ctx.run();
            }

            static void setup(vivianite::renderer* r_ctx, void* ctx) {
                auto* e_ctx = (vivianite::engine*)ctx;

                vivianite::mesh cube_obj = vivianite::load_obj("assets/cube.obj");
                GLuint cube = r_ctx->upload_mesh(cube_obj.vertices);

                cube_obj.vao = cube;

                vivianite::model cube_model = {.obj=cube_obj, .position=glm::vec3(0.0f, 0.0f, 0.0f)};

                r_ctx->render_queue.push_back(cube_model);

                r_ctx->vsync = VIVIANITE_VSYNC_TRUE;
                r_ctx->apply_settings();

                glfwSetKeyCallback(r_ctx->window, e_ctx->key_callback);
                printf("SETUP\n");
            }

            static void update(vivianite::renderer* r_ctx, void* ctx) {
                auto* e_ctx = (vivianite::engine*)ctx;

                glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                if (e_ctx->keys[GLFW_KEY_ESCAPE] == true) {
                    r_ctx->exit();
                }
            }

            static void exit(vivianite::renderer* r_ctx, void* ctx) {
                auto* e_ctx = (vivianite::engine*)ctx;

                printf("%f FPS (%f ms)\n", 1 / r_ctx->delta_time, r_ctx->delta_time * 1000.0f);
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
