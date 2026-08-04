#include "include.hpp"

namespace vivianite {
    void renderer::error_callback(int error, const char* description) {
        fprintf(stderr, "Error: %s\n", description);
    }

    renderer::renderer(Logging* l_ctx): logger(l_ctx) {
        if (!glfwInit()) {
            this->init_status = false;
            logger->log(logger->FATAL, "Failed to initialize GLFW");
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

    bool renderer::read_shaders() {
        logger->log(logger->INFO, "Reading GLSL shaders");

        // Frag
        std::ifstream frag_file(this->program.frag_path);

        if (!frag_file.is_open()) {
            logger->log(logger->FATAL, "Failed to read Fragment shader");
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
            logger->log(logger->FATAL, "Failed to read Vertex shader");
            return false;
        }

        this->program.vert_raw = std::string(
            (std::istreambuf_iterator<char>(vert_file)),
            std::istreambuf_iterator<char>()
        );

        vert_file.close();

        return true;
    }

    void renderer::create_shaders() {
        logger->log(logger->INFO, "Creating shader program");

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
            logger->log(logger->ERROR, "Fragment shader error:\n{}", infoLog);

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
            logger->log(logger->ERROR, "Vertex shader error:\n{}", infoLog);

            glDeleteShader(this->program.frag); // Don't leak the shader.
            return;
        }

        this->program.program = glCreateProgram();

        glAttachShader(this->program.program, this->program.vert);
        glAttachShader(this->program.program, this->program.frag);

        glLinkProgram(this->program.program);

        GLint isLinked = 0;
        glGetProgramiv(this->program.program, GL_LINK_STATUS, &isLinked);

        if (isLinked == GL_FALSE) {
            char infoLog[512];
            glGetProgramInfoLog(this->program.program, 512, nullptr, infoLog);

            logger->log(logger->FATAL, "Shader program link error:\n{}", infoLog);

            glDeleteProgram(this->program.program);
            return;
        }
    }

    void renderer::create_depth_program(std::string path) {
        logger->log(logger->INFO, "Creating depth shader");

        std::ifstream compute_file(path);

        if (!compute_file.is_open()) {
            logger->log(logger->ERROR, "Failed to read depth shader");
            return;
        }

        std::string contents = std::string(
            (std::istreambuf_iterator<char>(compute_file)),
            std::istreambuf_iterator<char>()
        );

        const char* contents_cstr = contents.c_str();

        compute_file.close();

        GLuint shader = glCreateShader(GL_VERTEX_SHADER);

        glShaderSource(shader, 1, &contents_cstr, nullptr);        
        glCompileShader(shader);

        GLint isCompiled = 0;
        char infoLog[512];

        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if(isCompiled == GL_FALSE) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            logger->log(logger->ERROR, "Depth shader error:\n{}", infoLog);

            glDeleteShader(shader); // Don't leak the shader.
            return;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, shader);
        glLinkProgram(program);

        GLint isLinked = 0;

        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

        if (isLinked == GL_FALSE) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);

            logger->log(logger->ERROR, "Depth program error:\n{}", infoLog);

            glDeleteProgram(program);
            glDeleteShader(shader);

            return;
        }

        glDeleteShader(shader);

        this->depth_program = program;
    }

    GLuint renderer::create_compute_program(std::string path) {
        logger->log(logger->INFO, "Creating compute shader");

        std::ifstream compute_file(path);

        if (!compute_file.is_open()) {
            logger->log(logger->ERROR, "Failed to read Compute shader");
            return 0;
        }

        std::string contents = std::string(
            (std::istreambuf_iterator<char>(compute_file)),
            std::istreambuf_iterator<char>()
        );

        const char* contents_cstr = contents.c_str();

        compute_file.close();

        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);

        glShaderSource(shader, 1, &contents_cstr, nullptr);        
        glCompileShader(shader);

        GLint isCompiled = 0;
        char infoLog[512];

        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if(isCompiled == GL_FALSE) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            logger->log(logger->ERROR, "Compute shader error:\n{}", infoLog);

            glDeleteShader(shader); // Don't leak the shader.
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, shader);
        glLinkProgram(program);

        glDeleteShader(shader);

        GLint isLinked = 0;

        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);

        if (isLinked == GL_FALSE) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);

            logger->log(logger->ERROR, "Compute program error:\n{}", infoLog);

            glDeleteProgram(program);
            glDeleteShader(shader);

            return 0;
        }

        return program;
    }

    GLuint renderer::upload_mesh(const std::vector<float> &mesh) {
        GLuint vao;
        GLuint vbo;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(float), mesh.data(), GL_STATIC_DRAW);

        // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // UV
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(3);

        return vao;
    }

    void renderer::init_SSBOs() {
        logger->log(logger->INFO, "Uploading SSBOs");

        // Lights
        glGenBuffers(1, &light_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_ssbo);

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

        // Tiles
        uint32_t tiles_x = (width + 15) / 16;
        uint32_t tiles_y = (height + 15) / 16;
        uint32_t tile_count = tiles_x * tiles_y;

        this->tiles.resize(tile_count);

        for (uint32_t i = 0; i < tile_count; i++) {
            this->tiles[i].count = 0;
            this->tiles[i].offset = i * MAX_LIGHTS_PER_TILE;
        }

        glGenBuffers(1, &tile_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, tile_ssbo);

        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            tiles.size() * sizeof(tile),
            tiles.data(),
            GL_DYNAMIC_DRAW
        );

        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER,
            1,
            tile_ssbo
        );

        // Light indices
        std::vector<uint32_t> light_indices(
            tile_count * MAX_LIGHTS_PER_TILE,
            0
        );

        glGenBuffers(1, &tile_light_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, tile_light_ssbo);

        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            light_indices.size() * sizeof(uint32_t),
            light_indices.data(),
            GL_DYNAMIC_DRAW
        );

        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER,
            2,
            tile_light_ssbo
        );


        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void renderer::init_FBOs() {
        glGenFramebuffers(1, &depth_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT32F,
            width,
            height,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            depth_texture,
            0
        );

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Depth FBO incomplete\n";
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderer::upload_lights() {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->light_ssbo);

        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            this->lights.size() * sizeof(light),
            this->lights.data(),
            GL_DYNAMIC_DRAW
        );

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    bool renderer::initialize() {
        logger->log(logger->INFO, "Initializing OpenGL and GLAD");
        // OpenGL 4.6 CORE
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->gl_major_version);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, this->gl_minor_version);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // GLFW window
        this->window = glfwCreateWindow(this->width, this->height, this->title, NULL, NULL);

        if (this->window == NULL) {
            logger->log(logger->FATAL, "Failed to open Window");
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

        GLFWimage icon;

        icon.pixels = stbi_load(
            "assets/logo.png",
            &icon.width,
            &icon.height,
            nullptr,
            4
        );

        if (icon.pixels == nullptr) {
            Logging().log(Logging::ERROR, "Failed to load icon: {}", stbi_failure_reason());
        }
        else {
            glfwSetWindowIcon(window, 1, &icon);
            stbi_image_free(icon.pixels);
        }

        return true;
    }

    void renderer::apply_settings() {
        logger->log(logger->NOTICE, "Applying GLFW settings");

        glfwSetWindowTitle(window, this->title);
        glfwSetWindowSize(window, this->width, this->height);
        glfwSwapInterval(this->vsync);

        glUseProgram(this->program.program);
        glUniformMatrix4fv(
            glGetUniformLocation(this->program.program, "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(this->projection)
        );
    }

    void renderer::run() {
        logger->log(logger->INFO, "Starting main setup");
        this->setup_func();

        logger->log(logger->INFO, "Assigning shader program");
        glUseProgram(this->program.program);

        glUniformMatrix4fv(
            glGetUniformLocation(this->program.program, "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(this->projection)
        );

        this->last = glfwGetTime();

        logger->log(logger->INFO, "Scheduling main update loop");
    }

    void renderer::render_depth_buffer() {
        if (depth_program == 0) {
            logger->log(logger->FATAL, "Depth program was not created");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

        glUseProgram(depth_program);

        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glm::mat4 view = glm::mat4(1.0f);

        view = glm::rotate(
            view,
            -this->camera_rot.x,
            glm::vec3(1,0,0)
        );

        view = glm::rotate(
            view,
            -this->camera_rot.y,
            glm::vec3(0,1,0)
        );

        view = glm::rotate(
            view,
            -this->camera_rot.z,
            glm::vec3(0,0,1)
        );

        view = glm::translate(
            view,
            -this->camera_pos
        );

        glUniformMatrix4fv(
            glGetUniformLocation(this->depth_program, "view"),
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(this->depth_program, "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(this->projection)
        );

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
                glGetUniformLocation(this->depth_program, "model"),
                1,
                GL_FALSE,
                glm::value_ptr(modelMat)
            );

            glDrawArrays(GL_TRIANGLES, 0, obj.obj.vertex_count);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(
            0,
            0,
            this->width,
            this->height
        );

        glBindTexture(GL_TEXTURE_2D, depth_texture);
    }

    void renderer::render_update() {
        this->shutdown = glfwWindowShouldClose(this->window);
            

        this->time = glfwGetTime();
        this->delta_time = this->time - this->last;
        this->last = this->time;

        this->update_func();

        // Camera rotation and position
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::rotate(
            view,
            -this->camera_rot.x,
            glm::vec3(1,0,0)
        );

        view = glm::rotate(
            view,
            -this->camera_rot.y,
            glm::vec3(0,1,0)
        );

        view = glm::rotate(
            view,
            -this->camera_rot.z,
            glm::vec3(0,0,1)
        );

        view = glm::translate(
            view,
            -this->camera_pos
        );

        glUseProgram(this->program.program);

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
        glUniform1ui(
            glGetUniformLocation(this->program.program, "light_count"),
            static_cast<unsigned int>(this->lights.size())
        );

        glUniform2f(
            glGetUniformLocation(this->program.program, "screen_size"),
            this->width,
            this->height
        );

        // Get depth buffer 
        this->render_depth_buffer();

        glMemoryBarrier(
            GL_FRAMEBUFFER_BARRIER_BIT |
            GL_TEXTURE_FETCH_BARRIER_BIT
        );

        // Light culling
        glUseProgram(this->tile_culling_init_program);
        glBindTextureUnit(0, this->depth_texture);

        glUniform1i(
            glGetUniformLocation(this->tile_culling_init_program, "depth_texture"),
            0
        );

        GLint loc_tx = glGetUniformLocation(this->tile_culling_init_program, "tiles_x");
        GLint loc_ty = glGetUniformLocation(this->tile_culling_init_program, "tiles_y");

        uint32_t tiles_x = (this->width + 15) / 16;
        uint32_t tiles_y = (this->height + 15) / 16;

        // this->logger->log(this->logger->DEBUG,
        //     "tiles_x loc=%d tiles_y loc=%d, values: %u %u %d",
        //     loc_tx, loc_ty, tiles_x, tiles_y, (int)this->lights.size());

        glUniform1ui(loc_tx, tiles_x);
        glUniform1ui(loc_ty, tiles_y);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->light_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->tile_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->tile_light_ssbo);

        glUniform1ui(
            glGetUniformLocation(this->tile_culling_init_program, "max_lights_per_tile"),
            MAX_LIGHTS_PER_TILE
        );

        glDispatchCompute(
            (tiles_x * tiles_y + 255) / 256,
            1,
            1
        );

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glUseProgram(this->tile_culling_program);
        
        glBindTextureUnit(0, this->depth_texture);

        glUniform1i(
            glGetUniformLocation(this->tile_culling_program, "depth_texture"),
            0
        );

        loc_tx = glGetUniformLocation(this->tile_culling_program, "tiles_x");
        loc_ty = glGetUniformLocation(this->tile_culling_program, "tiles_y");
        GLint loc_lc = glGetUniformLocation(this->tile_culling_program, "light_count");

        glUniform1ui(loc_tx, tiles_x);
        glUniform1ui(loc_ty, tiles_y);
        glUniform1ui(loc_lc, (unsigned int)this->lights.size());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->light_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->tile_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->tile_light_ssbo);

        glBindTextureUnit(
            1,
            this->depth_texture
        );

        glUniform1ui(
            glGetUniformLocation(this->tile_culling_program, "max_lights_per_tile"),
            MAX_LIGHTS_PER_TILE
        );
        glUniform1ui(
            glGetUniformLocation(this->tile_culling_program, "light_count"),
            static_cast<unsigned int>(this->lights.size())
        );

        glDispatchCompute(
            tiles_x,
            tiles_y,
            1
        );

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->tile_ssbo);

        tile* ptr = (tile*)glMapBuffer(GL_SHADER_STORAGE_BUFFER,GL_READ_ONLY);

        if (ptr) {
            uint32_t mid_tile = (tiles_y / 2) * tiles_x + (tiles_x / 2);

            // this->logger->log(this->logger->DEBUG, "Tile mid (%u) count: %d", mid_tile, ptr[mid_tile].count);

            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }

        // Actually Render
        glUseProgram(this->program.program);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->light_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->tile_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->tile_light_ssbo);

        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER,
            0,
            this->light_ssbo
        );

        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER,
            1,
            this->tile_ssbo
        );

        glBindBufferBase(
            GL_SHADER_STORAGE_BUFFER,
            2,
            this->tile_light_ssbo
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

            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.mat.albedo->texture);

            // Material
            glUniform1i(
                glGetUniformLocation(this->program.program, "albedo_tex"),
                0
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

        glfwSwapBuffers(this->window);
    }

    void renderer::exit() {
        glfwSetWindowShouldClose(this->window, GLFW_TRUE);
    }

    renderer::~renderer() {
        logger->log(logger->NOTICE, "Exiting... Check log for errors.");
        this->exit_func();

        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    bool renderer::check_status() {
        return this->init_status;
    }
};
