#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"

#define STB_VORBIS_HEADER_ONLY
#include "../../external/stb/stb_vorbis.c"

#define STB_VORBIS_IMPLEMENTATION
extern "C" {
#include "../../external/stb/stb_vorbis.c"
}

#define DR_WAV_IMPLEMENTATION
#include "../../external/dr_libs/dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "../../external/dr_libs/dr_mp3.h"
