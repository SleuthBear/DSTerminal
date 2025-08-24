//
// Created by Dylan Beaumont on 22/8/2025.
//

#include "MenuScreen.h"
unsigned int MenuScreen::tex = 0;
MenuScreen::MenuScreen(int *width, int *height) : UI(width, height) {
    buttons.push_back({
        {0.4, 0.3},
        {0.6, 0.45},
        {0.325f, 0.0f},
        {0.521f, 0.117f},
        [this](){this->exited = true;},
        });
    buttons.push_back({
        {0.4, 0.5},
        {0.6, 0.65},
        {0.522f, 0.0f},
        {0.749f, 0.126f},
        [this](){this->shouldClose = true;},
        });
}