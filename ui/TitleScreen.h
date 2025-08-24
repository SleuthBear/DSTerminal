//
// Created by Dylan Beaumont on 22/8/2025.
//

#ifndef DSTERMINAL_TITLESCREEN_H
#define DSTERMINAL_TITLESCREEN_H
#include "UI.h"
#include "../utils/TextureLoader.h"

class TitleScreen : public UI {
public:
    static unsigned int tex;
    TitleScreen(int *width, int *height);
    unsigned int getTex() override {return tex;};
};

#endif //DSTERMINAL_TITLESCREEN_H