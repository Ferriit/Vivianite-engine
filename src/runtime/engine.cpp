#define STB_IMAGE_IMPLEMENTATION
#include "include.hpp"

vivianite::Time::timestamp elapsed_time = 0;
vivianite::Time::timestamp start_time = 0;

int frame_count = 0;

namespace vivianite {
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
        elapsed_time = Time().get_time();

        r_ctx.initialize();

        i_ctx.initialize();

        i_ctx.set_axis("k_vertical_axis", (Axis){
            .keys = std::optional(std::pair<KeyType, KeyType>(KeyType::K_w, KeyType::K_s)),
            .analog = std::optional(KeyType::C_left_y),
            .mouse_analog = std::nullopt
        });

        i_ctx.set_axis("k_horizontal_axis", (Axis){
            .keys = std::optional(std::pair<KeyType, KeyType>(KeyType::K_d, KeyType::K_a)),
            .analog = std::optional(KeyType::C_left_x),
            .mouse_analog = std::nullopt
        });

        i_ctx.set_axis("m_vertical_axis", (Axis){
            .keys = std::nullopt,
            .analog = std::optional(KeyType::C_right_y),
            .mouse_analog = std::optional(KeyType::M_y)
        });

        i_ctx.set_axis("m_horizontal_axis", (Axis){
            .keys = std::nullopt,
            .analog = std::optional(KeyType::C_right_x),
            .mouse_analog = std::optional(KeyType::M_x)
        });

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

        Scheduler::Task fixed_loop_tks = (Scheduler::Task) {
            .run_type = Scheduler::Task::INTERVAL,
            .interval=20,
            .multi_threaded=true,
            .callback = [this]() {
                this->fixed_update();
            },
        };
        this->s_ctx.add_task(fixed_loop_tks);

        r_ctx.run();
    }

    void engine::setup() {
        // Set up cube
        // Set up Textures
        ResourceID cube_tex_id = rm_ctx.add_obj<Texture>("assets/cube.png");
        rm_ctx.data[cube_tex_id] =
            std::static_pointer_cast<void>(
                std::make_shared<Texture>(Texture{
                    .wraping = GL_REPEAT,
                    .min_filtering = GL_NEAREST,
                    .mag_filtering = GL_NEAREST
                }
            )
        );

        rm_ctx.tex_load_obj(cube_tex_id);

        ResourceID cube_id = rm_ctx.add_obj<mesh>("assets/cube.obj");
        
        rm_ctx.obj_load_obj(cube_id);

        vivianite::mesh cube_obj = *rm_ctx.get_obj<mesh>(cube_id);
        GLuint cube = r_ctx.upload_mesh(cube_obj.vertices);

        cube_obj.vao = cube;

        vivianite::model cube_model = {.obj=cube_obj, .position=glm::vec3(0.0f, 0.0f, 0.0f), .rotation=glm::vec3(0.0f, 0.0f, 0.0f)};

        cube_model.mat = (vivianite::material){
            .albedo = rm_ctx.get_obj<Texture>(cube_tex_id),
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

        l_ctx.log(Logging::log_level::NOTICE, "ENGINE SETUP DONE. TOOK {}ms", Time().get_time() - elapsed_time);
    }

    void engine::update() {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate
        r_ctx.render_queue[0].rotation.y += 0.03f;  
        r_ctx.render_queue[0].rotation.x += 0.03f;
        r_ctx.render_queue[0].rotation.z += 0.03f;

        l_ctx.log(Logging::log_level::DEBUG, "Mouse: {} {} {}", i_ctx.is_pressed(KeyType::M_lmb), i_ctx.is_pressed(KeyType::M_mmb), i_ctx.is_pressed(KeyType::M_rmb));

        r_ctx.camera_pos = glm::vec3(
            i_ctx.get_axis("k_horizontal_axis"),
            i_ctx.get_axis("k_vertical_axis"),
            5.0f
        );

        r_ctx.camera_rot += glm::vec3(
            i_ctx.get_axis("m_vertical_axis") / 10.0f,
            -i_ctx.get_axis("m_horizontal_axis") / 10.0f,
            0.0f
        );

        if (i_ctx.is_pressed(KeyType::K_escape)) {
            l_ctx.log(l_ctx.ERROR, "Shutdown: ESC pressed");
        }

        if (r_ctx.shutdown) {
            l_ctx.log(l_ctx.ERROR, "Shutdown: GLFW window close");
        }

        if (i_ctx.is_pressed(KeyType::K_escape) || r_ctx.shutdown) {
            this->s_ctx.running = false;
            r_ctx.exit();
        }

        frame_count++;
    }

    void engine::fixed_update() {
        l_ctx.log(Logging::log_level::DEBUG, "Fixed Update. elapsed_time={}", Time().get_time() - elapsed_time);

        elapsed_time = Time().get_time();
    }

    void engine::exit() {
        l_ctx.log(Logging::log_level::INFO, "{} FPS ({} ms)", frame_count / ((Time().get_time() - start_time) * 1000), ((Time().get_time() - start_time) * 1000) / frame_count);
    }
};


int main() {
    elapsed_time = vivianite::Time().get_time();
    start_time = vivianite::Time().get_time();

    vivianite::engine ctx;

    return ctx.status;
}
