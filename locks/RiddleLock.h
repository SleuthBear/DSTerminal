//
// Created by Dylan Beaumont on 26/8/2025.
//

#ifndef DSTERMINAL_RIDDLELOCK_H
#define DSTERMINAL_RIDDLELOCK_H
#include <string>
#include "../utils/KeyState.h"
#include "../utils/Shader.h"
#include "../utils/TextUtil.h"
#include "glfw3.h"
#include "../utils/CloseStates.h"
#include "../locks.h"
#include "../include/cglm/cglm.h"
#include "../utils/FileUtils.h"

struct LockConfig {
    std::string hint;
    std::string answer;
    bool fuzzy;
    Shader* shader;
    Character* characters;
    unsigned int atlasTex;
};

class RiddleLock {
public:
    FileNode** pos;
    Shader* shader;
    int* scrWidth;
    int* scrHeight;
    std::string hint;
    std::string answer;
    bool fuzzy;
    std::string input;
    bool active = false;
    int shouldClose = 0;
    int cursor = 0;
    vec3 color = GREY;
    std::string fileRef;
    Character *characters;
    unsigned int atlasTex=0;
    unsigned int VAO=0, VBO=0;
    RiddleLock(std::string fileRef, const LockConfig& config, int* width, int* height, FileNode** pos);
    int update(GLFWwindow* window, KeyState *keyState, double deltaTime);
    int processInput(GLFWwindow *window, KeyState *keyState, double deltaTime);
    void render();
    std::vector<int> getLineWraps(std::string_view text, float x, float width, float scale);
    void renderText(Shader &shader, std::string text, float xInitial, float y, std::vector<int> lineWraps, float width, float lineHeight, float scale, vec3 color);
    void delayClose(int milliseconds);
    void delayColorChange(int milliseconds, float color[3]);
};


#endif //DSTERMINAL_RIDDLELOCK_H