//
// Created by Dylan Beaumont on 22/8/2025.
//

#ifndef DSTERMINAL_MENUSCREEN_H
#define DSTERMINAL_MENUSCREEN_H
#include "UI.h"


class MenuScreen : public UI {
public:
    static unsigned int tex;
    MenuScreen(int *width, int *height);
    unsigned int getTex() override {return tex;};
};


#endif //DSTERMINAL_MENUSCREEN_H