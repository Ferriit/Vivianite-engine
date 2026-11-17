#include "main.hpp"
#include <iostream>

int main() {
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // OpenGL 4.6 COMPAT
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Vivianite Editor",
        INITIAL_WIDTH, INITIAL_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create SDL3 window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetWindowRelativeMouseMode(window, false);

    // Initialize OpenGL
    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    if (!glctx) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create OpenGL Context: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize OpenGL bindings
    GLenum glewRes = glewInit();
    if (glewRes != GLEW_OK) {
        SDL_Log("glewInit failed: %s\n", glewGetErrorString(glewRes));
        SDL_GL_DestroyContext(glctx);
        SDL_DestroyWindow(window);
        return 1;
    }

    // Initialize FreeType
    FT_Library ft;
    face;

    FT_Init_FreeType(&ft);

    FT_New_Face(
        ft,
        "external/MapleMono-NF-Regular.ttf",
        0,
        &face
    );
    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

    // Initialize MicroUI
    mu_init(&ctx);

    set_colors();

    ctx.text_width = text_width;
    ctx.text_height = text_height;

    // Set OpenGL to "pixel-mode"
    SDL_GetWindowSizeInPixels(window, &width, &height);

    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (glGetError() != GL_NO_ERROR);
    std::cout << glGetString(GL_VERSION) << std::endl;

    bool running = true;
    SDL_Event event;

    SDL_StartTextInput(window);

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    running = false;
                } break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    int mu_btn = 0;
                    if (event.button.button == SDL_BUTTON_LEFT)   mu_btn = MU_MOUSE_LEFT;
                    if (event.button.button == SDL_BUTTON_RIGHT)  mu_btn = MU_MOUSE_RIGHT;
                    if (event.button.button == SDL_BUTTON_MIDDLE) mu_btn = MU_MOUSE_MIDDLE;

                    if (mu_btn) {
                        mu_input_mousedown(&ctx, event.button.x, event.button.y, mu_btn);
                    }
                } break;
                case SDL_EVENT_MOUSE_BUTTON_UP: {
                    int mu_btn = 0;
                    if (event.button.button == SDL_BUTTON_LEFT)   mu_btn = MU_MOUSE_LEFT;
                    if (event.button.button == SDL_BUTTON_RIGHT)  mu_btn = MU_MOUSE_RIGHT;
                    if (event.button.button == SDL_BUTTON_MIDDLE) mu_btn = MU_MOUSE_MIDDLE;

                    if (mu_btn) {
                        mu_input_mouseup(&ctx, event.button.x, event.button.y, mu_btn);
                    }
                } break;
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
                    update_viewport(window);
                } break;
                case SDL_EVENT_TEXT_INPUT: {
                    printf("%s\n", event.text.text);
                } break;
            }
        }

        input_microui();
        
        mu_begin(&ctx);
        mu_Container* win = mu_get_container(&ctx, "Vivianite Editor");
        if (win) {
            win->rect = mu_rect(0, 0, width, height);
        }
        if (mu_begin_window(&ctx, "Vivianite Editor", mu_rect(0, 0, width, height))) {
            mu_layout_row(&ctx, 1, (int[]){ -1 }, 0);

            mu_label(&ctx, "Hello, World!");

            if (mu_button_ex(&ctx, "Click me!", 0, 0)) {
                std::cout << "Clicked!" << std::endl;
            }

            mu_end_window(&ctx);
        }
        else {
            running = false;
        }

        mu_end(&ctx);
        glClear(GL_COLOR_BUFFER_BIT);
        // Scissors for clipping
        glEnable(GL_SCISSOR_TEST);
        render(&ctx);

        SDL_GL_SwapWindow(window);
    }
    SDL_StopTextInput(window);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

