//
// Created by Dylan Beaumont on 22/8/2025.
//

#include "TitleScreen.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/type_ptr.inl"

unsigned int TitleScreen::tex = 0;
TitleScreen::TitleScreen(int* width, int* height, Terminal* terminal) : UI(width, height) {
    buttons.push_back({
        {0.4, 0.5},
        {0.6, 0.65},
        {0.325f, 0.0f},
        {0.521f, 0.117f},
        [this](){this->exited = true;},
        });
    buttons.push_back({
        {0.4, 0.65},
        {0.6, 0.80},
        {0.559f, 0.132f},
        {0.757f, 0.250f},
        [this, terminal](){terminal->loadGame(); this->exited = true;},
        });
    visuals.push_back({
            {0.25, 0.1},
            {0.75, 0.35},
            {0.0f, 0.0f},
            {0.318f, 0.24f},
    });
}