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
    struct Vertex {
        float x, y, z;
        float r, g, b;
    };

    std::vector<Vertex> load_obj(const char* filename)
    {
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

                int a, b, c;
                char slash;

                // Supports: f 1/1/1 2/2/2 3/3/3
                ss >> a >> slash >> slash >> b >> slash >> slash >> c;

                faces.push_back({a - 1, b - 1, c - 1});
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
        std::vector<Vertex> vertices;
        vertices.reserve(faces.size() * 3);

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

                vertices.push_back({
                    x, y, z,
                    r, g, b
                });
            }
        }

        return vertices;
    }
};

int frames = 0;

void setup(vivianite::renderer* ctx) {
    ctx->vsync = VIVIANITE_VSYNC_TRUE;
    ctx->apply_settings();
    printf("SETUP\n");
}

void update(vivianite::renderer* ctx) {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    frames++;

    if (frames >= 120) {
        printf("%fFPS\n", 1 / ctx->delta_time);
        ctx->exit();
    }
}

int main() {
    vivianite::renderer r_ctx;

    if (!r_ctx.check_status()) {
        return 1;
    }

    r_ctx.initialize();

    r_ctx.program.frag_path = "assets/frag.glsl";
    r_ctx.program.vert_path = "assets/vert.glsl";

    r_ctx.create_shaders();
    
    r_ctx.setup_func = setup;
    r_ctx.update_func = update;

    r_ctx.run();

    return 0;
}
