#pragma once
#include <chrono>
#include <ctime>
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <string>

#include "../external/glad/include/glad/gl.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "../external/glm/glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <bits/stdc++.h>

#include <cstdlib>
#include <cstring>
#include <sstream>

#include <format>
#include <deque>
#include <iostream>

#include <utility>

#include "../external/stb/stb_image.h"

#define VIVIANITE_VSYNC_TRUE 1
#define VIVIANITE_VSYNC_FALSE 0
#define VIVIANITE_VSYNC_HALF 2

#define MAX_LIGHTS_PER_TILE 64

namespace vivianite {
    struct shader {
        std::string frag_path, vert_path;
        std::string frag_raw, vert_raw;
        GLuint frag, vert, program;
    };

    struct texture {
        GLenum warping; // GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
        GLenum filtering; // GL_NEAREST, GL_LINEAR
        GLint texture;
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

    using ResourceID = uint64_t;

    class ResourceManager {
        private:
            std::vector<std::tuple<bool, std::shared_ptr<void>, std::string>> objects;

        public:
            template<typename T>
            void set_obj(ResourceID ID, std::string path) {
                objects[ID] = {
                    false,
                    nullptr,
                    path
                };
            }

            template<typename T>
            ResourceID add_obj(std::string path) {
                objects.push_back({
                    false,
                    nullptr,
                    path
                });
                return objects.size() - 1;
            }

            bool is_loaded(ResourceID ID) {
                if (ID >= objects.size())
                    return false;

                auto [loaded, obj, path] = objects[ID];
                return loaded;
            }

            template<typename T>
            std::shared_ptr<T> get_obj(ResourceID ID) {
                if (ID >= objects.size())
                    return nullptr;

                auto [loaded, obj, path] = objects[ID];
                return static_pointer_cast<T>(obj);
            }

            bool obj_load_obj(ResourceID ID);
    };

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

        bool verbose;

        Logging(bool colored = true)
            : colored(colored) {}

        template <typename... Args>
        void log(log_level level, std::format_string<Args...> format, Args&&... args) {
            if (level == log_level::DEBUG && !verbose) return;

            auto dt_UTC = Time().get_time();
            auto dt = Time().utc_to_date_time(dt_UTC);
            std::string message = std::format(format, std::forward<Args>(args)...);

            buffer.push_back({level, dt, message});
            if (buffer.size() > max_buffer_size)
                buffer.pop_front();

            static constexpr std::array<std::pair<std::string_view, std::string_view>, 7> level_info = {{
                {"INFO",   "\x1b[32m"},
                {"WARN",   "\x1b[38;5;214m"},
                {"ERROR",  "\x1b[31;1m"},
                {"FATAL",  "\x1b[31;40m"},
                {"NOTICE", "\x1b[34;1m"},
                {"DEBUG",  "\x1b[38;5;242m"},
                {"",       "\x1b[0m"},
            }};
            const auto& [label, color] = level_info[std::to_underlying(level)];

            if (colored) std::cout << color;
            if (!label.empty()) std::cout << "[" << label << "] ";
            std::cout << std::format("[{:04}-{:02}-{:02} {:02}:{:02}:{:02}] ",
                                      dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
            if (colored) std::cout << "\x1b[0m";
            std::cout << message << "\n" << std::flush;
        }

        void clear() { buffer.clear(); }

    private:
        std::deque<log_entry> buffer;
        std::size_t max_buffer_size;
        bool colored;
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
            std::function<void()> callback;

            void* e_ctx;
            void* r_ctx;
            
            void execute();
        };

        class Scheduler {
            private:
                std::vector<Task> tasks;
            
            public:
                void* r_ctx; // Renderer Reference

                Logging* l_ctx = nullptr;

                bool running = true;

                void add_task(Task tsk);

                void main_loop();
        };
    };

    enum KeyType {
        // Letters
        K_a, K_b, K_c, K_d, K_e, K_f, K_g,
        K_h, K_i, K_j, K_k, K_l, K_m, K_n,
        K_o, K_p, K_q, K_r, K_s, K_t, K_u,
        K_v, K_w, K_x, K_y, K_z,

        // Numbers
        K_0, K_1, K_2, K_3, K_4,
        K_5, K_6, K_7, K_8, K_9,

        // Function keys
        K_f1, K_f2, K_f3, K_f4, K_f5, K_f6,
        K_f7, K_f8, K_f9, K_f10, K_f11, K_f12,

        // Arrow keys
        K_up,
        K_down,
        K_left,
        K_right,

        // Navigation
        K_insert,
        K_delete,
        K_home,
        K_end,
        K_page_up,
        K_page_down,

        // Modifiers
        K_left_shift,
        K_right_shift,
        K_left_control,
        K_right_control,
        K_left_alt,
        K_right_alt,
        K_left_super,
        K_right_super,

        // Whitespace
        K_space,
        K_tab,
        K_enter,
        K_backspace,
        K_escape,

        // Symbols
        K_apostrophe,
        K_comma,
        K_minus,
        K_period,
        K_slash,
        K_semicolon,
        K_equal,
        K_left_bracket,
        K_right_bracket,
        K_backslash,
        K_grave_accent,

        // Lock keys
        K_caps_lock,
        K_scroll_lock,
        K_num_lock,

        // Numpad
        K_num_0,
        K_num_1,
        K_num_2,
        K_num_3,
        K_num_4,
        K_num_5,
        K_num_6,
        K_num_7,
        K_num_8,
        K_num_9,

        K_num_decimal,
        K_num_divide,
        K_num_multiply,
        K_num_subtract,
        K_num_add,
        K_num_enter,
        K_num_equal,

        // Misc
        K_print_screen,
        K_pause,

        // Controller buttons
        // Joystick slots
        C_JOYSTICK_1,
        C_JOYSTICK_2,
        C_JOYSTICK_3,
        C_JOYSTICK_4,
        C_JOYSTICK_5,
        C_JOYSTICK_6,
        C_JOYSTICK_7,
        C_JOYSTICK_8,
        C_JOYSTICK_9,
        C_JOYSTICK_10,
        C_JOYSTICK_11,
        C_JOYSTICK_12,
        C_JOYSTICK_13,
        C_JOYSTICK_14,
        C_JOYSTICK_15,
        C_JOYSTICK_16,

        // Face buttons
        C_a,
        C_b,
        C_x,
        C_y,

        // Shoulder buttons
        C_left_bumper,
        C_right_bumper,

        // Menu buttons
        C_back,
        C_start,
        C_guide,

        // Stick clicks
        C_left_thumb,
        C_right_thumb,

        // D-pad
        C_dpad_up,
        C_dpad_right,
        C_dpad_down,
        C_dpad_left,

        // Analog axes
        C_left_x,
        C_left_y,
        C_right_x,
        C_right_y,

        C_left_trigger,
        C_right_trigger,

        // PlayStation aliases
        C_cross = C_a,
        C_circle = C_b,
        C_triangle = C_y,
        C_square = C_x,

        K_unknown = C_right_trigger + 1
    };

    enum KeyAction {
        KEY_DOWN,
        KEY_UP,
        KEY_REPEAT
    };
    
    struct KeyEvent {
        KeyType key;
        KeyAction action;
        int mods;
    };

    struct Axis {
        std::optional<std::pair<KeyType, KeyType>> keys;
        std::optional<KeyType> joystick;
    };

    class Input {
        private:
            static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

            std::unordered_map<std::string, Axis> input_axes;
        public:
            std::vector<std::function<void(KeyEvent)>> key_callbacks;

                std::array<bool, KeyType::K_unknown + 1> keys;

            std::array<float, KeyType::C_right_trigger - KeyType::C_left_x + 1> analog_axes;

            float deadzone = 0.15f;
            
            void* r_ptr = nullptr;

            float apply_deadzone(float value);

            void initialize();
            void update();

            float get_axis(std::string name);

            void set_axis(std::string name, Axis axis);

            KeyType glfw_to_keytype(int key);
            KeyType glfw_gamepad_button_to_keytype(int button);
            int keytype_to_glfw_joystick(KeyType key);
            int keytype_to_glfw_gamepad_button(KeyType key);

            bool is_pressed(KeyType key);
    };

    // RENDERER
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

            std::function<void()> update_func;
            std::function<void()> setup_func;
            std::function<void()> exit_func;

            void* engine_ctx;
            Input* i_ctx = nullptr;

            std::vector<model> render_queue = {};

            glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 5.0f);
            glm::vec3 camera_rot = glm::vec3(0.0f, 0.0f, 0.0f);
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

            void render_update();

            void exit();

            ~renderer();

            bool check_status();
    };

    // ENGINE
    class engine {
        public:
            int status = 0;

            ResourceManager rm_ctx;

            std::array<bool, GLFW_KEY_LAST + 1> keys = {};
            std::unordered_set<int> scancodes = {};
            
            Logging l_ctx;

            vivianite::renderer r_ctx;

            Scheduler::Scheduler s_ctx;

            Input i_ctx;

            engine();

            void init();

            void setup();

            void update();

            void exit();

        private:
            void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    };
}
