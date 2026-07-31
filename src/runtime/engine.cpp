#include "include.hpp"


namespace vivianite {
    mesh load_obj(const char* filename) {
        Logging().log(Logging::INFO, "Reading file \"{}\"", filename);

        std::ifstream file(filename);

        if (!file.is_open()) {
            Logging().log(Logging::ERROR, "Unable to open model \"{}\"", filename);
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

    engine::engine() : r_ctx(&l_ctx) {
        s_ctx.r_ctx = (void*)(&this->r_ctx);

        s_ctx.l_ctx = &l_ctx;

        i_ctx.r_ptr = (void*)&r_ctx;
        
        Scheduler::Task e_init = (Scheduler::Task) {
            .run_type = Scheduler::Task::ASAP,
            .callback = [this]() {
                this->init();
            },
        };

        s_ctx.add_task(e_init);


        #ifndef DBG
        s_ctx.l_ctx->verbose = false;
        #endif
        #ifdef DBG
        s_ctx.l_ctx ->verbose = true;
        #endif

        s_ctx.main_loop();
    }

    void engine::init() {
        if (!r_ctx.check_status()) {
            this->status = 1;
            return;
        }

        r_ctx.initialize();

        i_ctx.initialize();

        r_ctx.i_ctx = &i_ctx;

        r_ctx.program.frag_path = "assets/frag.glsl";
        r_ctx.program.vert_path = "assets/vert.glsl";

        r_ctx.create_shaders();

        r_ctx.tile_culling_program = r_ctx.create_compute_program("assets/cull.comp");
        r_ctx.tile_culling_init_program = r_ctx.create_compute_program("assets/init_cull.comp");

        r_ctx.create_depth_program("assets/depth.vert");

        this->l_ctx.log(Logging::INFO, "Assigning functions");
        r_ctx.setup_func = [this]() {this->setup();};
        r_ctx.update_func = [this]() {this->update();};
        r_ctx.exit_func = [this]() {this->exit();};

        r_ctx.engine_ctx = this;
        
        Scheduler::Task input_loop_tks = (Scheduler::Task) {
            .run_type=Scheduler::Task::EVERY_FRAME,
            .callback=[this]() {
                i_ctx.update();
            },
        };
        this->s_ctx.add_task(input_loop_tks);

        Scheduler::Task main_loop_tsk = (Scheduler::Task) {
            .run_type=Scheduler::Task::EVERY_FRAME,
            .callback=[this]() {
                r_ctx.render_update();
            },
        };
        this->s_ctx.add_task(main_loop_tsk);

        r_ctx.run();
    }

    void engine::setup() {
        // Set up cube
        vivianite::mesh cube_obj = vivianite::load_obj("assets/cube.obj");
        GLuint cube = r_ctx.upload_mesh(cube_obj.vertices);

        cube_obj.vao = cube;

        vivianite::model cube_model = {.obj=cube_obj, .position=glm::vec3(0.0f, 0.0f, 0.0f), .rotation=glm::vec3(0.0f, 0.0f, 0.0f)};

        cube_model.mat = (vivianite::material){
            .albedo = glm::vec3(1.0f, 1.0f, 1.0f),
            .shininess = 32.0f,
            .specular_strength = 0.5f
        };

        l_ctx.log(Logging::INFO, "Uploading models");
        r_ctx.render_queue.push_back(cube_model);

        r_ctx.vsync = VIVIANITE_VSYNC_TRUE;
        r_ctx.apply_settings();

        l_ctx.log(Logging::INFO, "Uploading lights");
        // Set up lights
        r_ctx.lights.push_back(
            (vivianite::light){
                .position=glm::vec4(-5.0f, 0.0f, 0.0f, 1.0f),
                .color=glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
                .radius=50.0f,
                .strength=1.0f,
                .linear=0.09f,
                .quadratic=0.032f
            }
        );
        r_ctx.lights.push_back(
            (vivianite::light){
                .position=glm::vec4(5.0f, 0.0f, 0.0f, 1.0f),
                .color=glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
                .radius=50.0f,
                .strength=1.0f,
                .linear=0.09f,
                .quadratic=0.032f
            }
        );
        r_ctx.lights.push_back(
            (vivianite::light){
                .position=glm::vec4(0.0f, -5.0f, 0.0f, 1.0f),
                .color=glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
                .radius=50.0f,
                .strength=1.0f,
                .linear=0.09f,
                .quadratic=0.032f
            }
        );
        r_ctx.init_SSBOs();

        r_ctx.init_FBOs();

        l_ctx.log(Logging::log_level::NOTICE, "ENGINE SETUP DONE");
    }

    void engine::update() {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate
        r_ctx.render_queue[0].rotation.y += 0.03f;  
        r_ctx.render_queue[0].rotation.x += 0.03f;
        r_ctx.render_queue[0].rotation.z += 0.03f;

        if (i_ctx.is_pressed(KeyType::K_escape) || r_ctx.shutdown) {
            this->s_ctx.running = false;
            r_ctx.exit();
        }
    }

    void engine::exit() {
        l_ctx.log(Logging::log_level::INFO, "{} FPS ({} ms)", 1 / r_ctx.delta_time, r_ctx.delta_time * 1000.0f);
    }
};


int main() {
    vivianite::engine ctx;

    return ctx.status;
}
