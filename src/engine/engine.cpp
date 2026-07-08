#include "renderer.hpp"
#include <iostream>

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
