//
// Created by Dylan Beaumont on 22/8/2025.
//

#ifndef DSTERMINAL_MENUSCREEN_H
#define DSTERMINAL_MENUSCREEN_H
#include "UI.h"
#include "../utils/FileUtils.h"

class MenuScreen : public UI {
public:
    FileNode* node;
    static unsigned int tex;
    MenuScreen(int *width, int *height, FileNode *node);
    unsigned int getTex() override {return tex;};
    void save();
};


#endif //DSTERMINAL_MENUSCREEN_H