//
// Created by Dylan Beaumont on 13/8/2025.
//

#ifndef GAMELAYER_H
#define GAMELAYER_H
#include <functional>
#include <glfw3.h>
#include "KeyState.h"
// For now just has an update function. Could have more info later.
struct GameLayer {
    std::function<int(GLFWwindow*, KeyState*, double)> update;
    std::function<void()> cleanup;
};

#endif //GAMELAYER_H
