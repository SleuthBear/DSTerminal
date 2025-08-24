//
// Created by Dylan Beaumont on 20/8/2025.
//

#include "SoundManager.h"
#define MINIAUDIO_IMPLEMENTATION
#include <audio/miniaudio.h>
SoundManager::SoundManager() {
    ma_engine_init(nullptr, &engine);
    ma_result result;
    result = ma_sound_init_from_file(&engine, "../resources/type.wav", MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &type);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to load sound");
    }
}

// todo test if possible to have same sound overlapping. If not will need the low level api
// todo better sound samples
// todo music
void SoundManager::play() {
    ma_sound_stop(&type);
    ma_sound_seek_to_pcm_frame(&type, 0);
    ma_sound_start(&type);
}

