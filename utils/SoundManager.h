//
// Created by Dylan Beaumont on 20/8/2025.
//

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H
#include <audio/miniaudio.h>

class SoundManager {
public:
    ma_engine engine;
    ma_sound type;
    SoundManager();
    void play();

};



#endif //SOUNDMANAGER_H
