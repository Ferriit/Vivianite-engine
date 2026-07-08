#include "renderer.hpp"
#include <iostream>

int frames = 0;

void setup(vivianite::renderer* ctx) {
    frames = 0;
    printf("SETUP\n");
}

void update(vivianite::renderer* ctx) {
    frames++;

    if (frames >= 100) {
        printf("%fFPS\n", 1 / ctx->delta_time);
        ctx->exit();
    }
}

namespace vivianite {
    static inline int start() {
        renderer r_ctx;

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
};
