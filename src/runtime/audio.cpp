#include "include.hpp"
#include "../external/stb/stb_vorbis.c"

namespace vivianite {
    bool Audio::initialize() {
        l_ctx->log(Logging::INFO, "Initializing Audio system");

        this->device = alcOpenDevice(nullptr);

        if (!this->device) {
            l_ctx->log(Logging::ERROR, "Unable to get audio device");
            return false;
        }

        this->ctx = alcCreateContext(device, nullptr);

        if (!this->ctx) {
            l_ctx->log(Logging::ERROR, "Unable to create OpenAL context");
            return false;
        }

        if (!alcMakeContextCurrent(this->ctx)) {
            alcDestroyContext(this->ctx);
            alcCloseDevice(this->device);

            l_ctx->log(Logging::ERROR, "Unable to make OpenAL context current");
            return false;
        }

        return true;
    }
};

