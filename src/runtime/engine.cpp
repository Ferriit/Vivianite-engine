#include "include.hpp"
#define PI 3.14159265f

vivianite::Time::timestamp elapsed_time = 0;
vivianite::Time::timestamp start_time = 0;

int frame_count = 0;

namespace vivianite {
    engine::engine() : r_ctx(&l_ctx) {
        s_ctx.r_ctx = (void*)(&this->r_ctx);

        s_ctx.l_ctx = &l_ctx;

        i_ctx.r_ptr = (void*)&r_ctx;

        a_ctx.l_ctx = &l_ctx;
        
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

        a_ctx.initialize();

        i_ctx.set_axis("k_vertical_axis", (Axis){
            .keys = std::optional(std::vector<std::pair<KeyType, KeyType>>{
                {KeyType::K_w, KeyType::K_s}
            }),
            .analog = std::optional(KeyType::C_left_y),
            .mouse_analog = std::nullopt
        });

        i_ctx.set_axis("k_horizontal_axis", (Axis){
            .keys = std::optional(std::vector<std::pair<KeyType, KeyType>>{
                {KeyType::K_d, KeyType::K_a}
            }),
            .analog = std::optional(KeyType::C_left_x),
            .mouse_analog = std::nullopt
        });

        i_ctx.set_axis("m_vertical_axis", (Axis){
            .keys = std::nullopt,
            .analog = std::optional(KeyType::C_right_y),
            .mouse_analog = std::optional(KeyType::M_y),
            .time_scaled = true
        });

        i_ctx.set_axis("m_horizontal_axis", (Axis){
            .keys = std::nullopt,
            .analog = std::optional(KeyType::C_right_x),
            .mouse_analog = std::optional(KeyType::M_x),
            .time_scaled = true
        });

        i_ctx.set_axis("k_jump", (Axis){
            .keys = std::optional(std::vector<std::pair<KeyType, KeyType>>{
                {KeyType::K_space, KeyType::K_none},
                {KeyType::C_a, KeyType::K_none},
            }),
            .analog = std::nullopt,
            .mouse_analog = std::nullopt
        });

        i_ctx.set_axis("k_crouch", (Axis){
            .keys = std::optional(std::vector<std::pair<KeyType, KeyType>>{
                {KeyType::K_left_control, KeyType::K_none},
                {KeyType::C_b, KeyType::K_none},
            }),
            .analog = std::nullopt,
            .mouse_analog = std::nullopt
        });

        i_ctx.add_action("exit", KeyType::K_escape);
        i_ctx.add_action("exit", KeyType::C_start);

        r_ctx.i_ctx = &i_ctx;

        r_ctx.program.frag_path = "assets/shaders/frag.glsl";
        r_ctx.program.vert_path = "assets/shaders/vert.glsl";

        r_ctx.create_shaders();

        r_ctx.tile_culling_program = r_ctx.create_compute_program("assets/shaders/cull.comp");
        r_ctx.tile_culling_init_program = r_ctx.create_compute_program("assets/shaders/init_cull.comp");

        r_ctx.create_depth_program("assets/shaders/depth.vert");

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
        ResourceID song = rm_ctx.add_obj<Sound>("assets/sounds/ticktock.mp3");
        rm_ctx.sound_load_obj(song);

        AudioSource source = a_ctx.update_source((AudioSource){
            .x=0.0f,
            .y=0.0f,
            .z=0.0f,
            .gain=0.25f,
            .pitch=1.0f,
            .loop=AL_TRUE,
        });

        a_ctx.play_sound(source, *rm_ctx.get_obj<Sound>(song));

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

        r_ctx.vsync = VIVIANITE_VSYNC_FALSE;
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
        glm::vec3 old_rotation = r_ctx.camera_rot;

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Rotate
        r_ctx.render_queue[0].rotation.y += 1.0f * r_ctx.delta_time;  
        r_ctx.render_queue[0].rotation.x += 1.0f * r_ctx.delta_time;
        r_ctx.render_queue[0].rotation.z += 1.0f * r_ctx.delta_time;

        r_ctx.camera_rot += glm::vec3(
            i_ctx.get_axis("m_vertical_axis") / 10.0f,
            -i_ctx.get_axis("m_horizontal_axis") / 10.0f,
            0.0f
        );

        if ((r_ctx.camera_rot.x > (PI / 2.0f)) || (r_ctx.camera_rot.x < -(PI / 2.0f)))
            r_ctx.camera_rot = old_rotation;

        glm::mat4 rot(1.0f);
        rot = glm::rotate(rot, r_ctx.camera_rot.x, glm::vec3(1,0,0));
        rot = glm::rotate(rot, r_ctx.camera_rot.y, glm::vec3(0,1,0));
        rot = glm::rotate(rot, r_ctx.camera_rot.z, glm::vec3(0,0,1));

        glm::vec3 forward = glm::normalize(glm::vec3(rot * glm::vec4(0, 0, -1, 0)));
        glm::vec3 right = glm::normalize(glm::vec3(rot * glm::vec4(1, 0, 0, 0)));
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::vec3 down = -up;

        r_ctx.camera_pos += forward * static_cast<float>(r_ctx.delta_time) * 5.0f * i_ctx.get_axis("k_vertical_axis");
        r_ctx.camera_pos += right * static_cast<float>(r_ctx.delta_time) * 5.0f * i_ctx.get_axis("k_horizontal_axis");
        #ifndef _WIN32
        r_ctx.camera_pos += up * static_cast<float>(r_ctx.delta_time) * 5.0f * i_ctx.get_axis("k_jump");
        r_ctx.camera_pos += down * static_cast<float>(r_ctx.delta_time) * 5.0f * i_ctx.get_axis("k_crouch");
        #endif
        #ifdef _WIN32
        r_ctx.camera_pos += down * static_cast<float>(r_ctx.delta_time) * 5.0f * i_ctx.get_axis("k_jump");
        r_ctx.camera_pos += up * static_cast<float>(r_ctx.delta_time) * 5.0f * i_ctx.get_axis("k_crouch");
        #endif

        if (i_ctx.get_action("exit")) {
            l_ctx.log(l_ctx.ERROR, "Shutdown: ESC pressed");
        }

        if (r_ctx.shutdown) {
            l_ctx.log(l_ctx.ERROR, "Shutdown: GLFW window close");
        }

        if (i_ctx.get_action("exit") || r_ctx.shutdown) {
            this->s_ctx.running = false;
            r_ctx.exit();
        }

        frame_count++;
    }

    void engine::fixed_update() {
        elapsed_time = Time().get_time();
    }

    void engine::exit() {
        double elapsed = Time().get_time() - start_time;

        l_ctx.log(
            Logging::log_level::INFO,
            "{} FPS ({} ms)",
            static_cast<double>(frame_count) / (elapsed / 1000.0),
            elapsed / static_cast<double>(frame_count)
        );
    }
};


int main() {
    elapsed_time = vivianite::Time().get_time();
    start_time = vivianite::Time().get_time();

    vivianite::engine ctx;

    return ctx.status;
}
