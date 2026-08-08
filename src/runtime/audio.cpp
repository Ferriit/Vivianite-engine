#include "include.hpp"
#include "../external/stb/stb_vorbis.c"

namespace vivianite {
    bool Audio::initialize() {
        l_ctx->log(Logging::INFO, "Initializing Audio system");

        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

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

    std::vector<std::string> Audio::get_output_devices() {
        std::vector<std::string> devices;

        const ALCchar* device_list = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

        if (!device_list)
            return devices;

        const ALCchar* current = device_list;

        while (*current != '\0') {
            devices.emplace_back(current);
            current += devices.back().size() + 1;
        }

        return devices;
    }

    AudioSource Audio::update_source(AudioSource templ) {
        ALuint source;
        alGenSources(1, &source);

        ALenum err = alGetError();
        if (err != AL_NO_ERROR) {
            l_ctx->log(Logging::ERROR, "Unable to create source: {}", err);
            return templ;
        }

        alSourcef(source, AL_GAIN, templ.gain);
        alSourcef(source, AL_PITCH, templ.pitch);

        alSource3f(
            source,
            AL_POSITION,
            templ.x,
            templ.y,
            templ.z
        );

        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcei(source, AL_LOOPING, templ.loop ? AL_TRUE : AL_FALSE);

        templ.source = source;

        return templ;
    }

    void Audio::play_sound(AudioSource src, Sound snd) {
        alSourcei(src.source, AL_BUFFER, snd);
        alSourcePlay(src.source);
    }

    void Audio::stop_sound(AudioSource src) {
        alSourceStop(src.source);
    }

    void Audio::pause_sound(AudioSource src) {
        alSourcePause(src.source);
    }

    void Audio::delete_source(AudioSource src) {
        alDeleteSources(1, &src.source);
    }
};

