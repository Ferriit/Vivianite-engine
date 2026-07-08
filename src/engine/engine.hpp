#include "renderer.hpp"

namespace vivianite {
    static inline int start() {
        renderer r_ctx;

        if (!r_ctx.check_status()) {
            return 1;
        }

        r_ctx.initialize();

        return 0;
    }
};
