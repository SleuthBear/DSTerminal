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




class RiddleLock {
public:
    Shader* shader;
    int* scrWidth;
    int* scrHeight;
    std::string hint;
    std::string answer;
    bool fuzzy;
    std::string input;
    bool active = false;
    int cursor = 0;
    KeyState *keyState;
    std::string fileRef;
    Character *characters;
    unsigned int atlasTex;
    unsigned int VAO, VBO;
    RiddleLock(std::string fileRef, LockInfo info, int* width, int* height);
    int update(GLFWwindow* window, KeyState *keyState, double deltaTime);
    int processInput(GLFWwindow *window, KeyState *keyState, double deltaTime);
    void render();
    std::vector<int> getLineWraps(std::string_view text, float x, float width, float scale);
    void renderText(Shader &shader, std::string text, float xInitial, float y, std::vector<int> lineWraps, float width, float lineHeight, float scale, vec3 color);

};


#endif //DSTERMINAL_RIDDLELOCK_H