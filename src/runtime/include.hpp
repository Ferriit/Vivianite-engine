#pragma once
#include <chrono>
#include <ctime>
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <string>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <bits/stdc++.h>

#define VIVIANITE_VSYNC_TRUE 1
#define VIVIANITE_VSYNC_FALSE 0
#define VIVIANITE_VSYNC_HALF 2

#define MAX_LIGHTS_PER_TILE 64

namespace vivianite {
    class Time {
    public:
        using timestamp = long long;

        struct date {
            int year;
            int month;
            int day;
        };

        struct clock {
            int hour;
            int min;
            int sec;
        };

        struct date_time {
            int year;
            int month;
            int day;
            int hour;
            int min;
            int sec;
            int millisec;
        };

        float delta_time = 0.0f;

        timestamp get_time();

        date get_date_utc();

        clock get_clock_utc();

        date_time get_date_time_utc();

        timestamp get_timezone_offset();

        date utc_to_date(timestamp ts);

        clock utc_to_clock(timestamp ts);

        date_time utc_to_date_time(timestamp ts);

        timestamp date_to_utc(date d);

        timestamp clock_to_utc(clock c);

        timestamp date_time_to_utc(date_time dt);
    };

    class Logging {
        public:
        enum log_level {
            INFO,
            WARNING,
            ERROR,
            FATAL,
            NOTICE,
            DEBUG,
            UNMARKED,
        };

        struct log_entry {
            log_level level;
            Time::date_time timestamp;
            std::string message;
        };

        bool colored;

        bool verbose = false;

        std::vector<log_entry> buffer;

        size_t max_buffer_size = 1000;

        Logging(bool colored = true)
            : colored(colored) {}

        void log(log_level level, const char* format, ...); 

        void clear(); 
    };

    namespace Scheduler {
        struct Task {
            enum TASK_TYPE {
                ASAP,
                ONCE,
                ABS,
                INTERVAL,
                EVERY_FRAME
            };

            TASK_TYPE run_type;

            Time::timestamp next_run;
            Time::timestamp interval;
            Time::timestamp delay;

            // ENGINE REFERENCE ↓
            void (*callback)(void*, void*);

            void* e_ctx;
            void* r_ctx;
            
            void execute();
        };

        class Scheduler {
            private:
                std::vector<Task> tasks;
            
            public:
                void* e_ctx; // Engine Reference
                void* r_ctx; // Renderer Reference

                Logging* l_ctx = nullptr;

                bool running = true;

                void add_task(Task tsk);

                void main_loop();
        };
    };

    class Input {
        public:
            // e_ctx, r_ctx
            std::vector<void (*)(void*, void*)> key_callbacks;
            
            void* r_ptr = nullptr;
            void* e_ptr = nullptr;

            Input();

            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    };

    // RENDERER
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

    struct tile {
        uint32_t count;
        uint32_t offset;
    };

    static_assert(sizeof(tile) == 8);

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
            GLuint depth_program;
            GLuint tile_culling_program;
            GLuint tile_culling_init_program;

            int gl_major_version = 4;
            int gl_minor_version = 6;

            void (*update_func)(vivianite::renderer*, void*);
            void (*setup_func)(vivianite::renderer*, void*);
            void (*exit_func)(vivianite::renderer*, void*);

            void* engine_ctx;
            Input* i_ctx = nullptr;

            std::vector<model> render_queue = {};

            glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 5.0f);
            glm::vec3 camera_rot = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::mat4 projection;

            std::vector<light> lights = {};
            std::vector<tile> tiles = {};

            GLuint light_ssbo;
            GLuint tile_light_ssbo;
            GLuint tile_ssbo;

            GLuint depth_fbo;
            GLuint depth_texture;

            double delta_time = 0.0;
            double time = 0.0;
            double last = 0.0;

            Logging* logger = nullptr;

            bool shutdown = false;

            static void error_callback(int error, const char* description);

            renderer(Logging* l_ctx);

            bool read_shaders();

            void create_shaders();

            void create_depth_program(std::string path);

            GLuint create_compute_program(std::string path);

            GLuint upload_mesh(const std::vector<float> &mesh);

            void init_SSBOs();

            void init_FBOs();

            void upload_lights();

            bool initialize();

            void apply_settings();

            void run();

            void render_depth_buffer();

            static void render_update(void* engine, void* renderer);

            void exit();

            ~renderer();

            bool check_status();
    };
}
