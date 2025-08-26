//
// Created by Dylan Beaumont on 12/8/2025.
//

#ifndef CYLINDERLOCK_H
#define CYLINDERLOCK_H
#include "../utils/Shader.h"
#include <glfw3.h>
#include <vector>

#include "../utils/KeyState.h"
#include "../terminal/Terminal.h"
#include "../utils/CloseStates.h"

#define MAX_SEGMENTS 64
struct vertex {
    float x, y, z;
};

class CylinderLock {
public:
    int* width;
    int* height;

    int segments;
    int sunkSegments[MAX_SEGMENTS]={};
    glm::vec3 color = {0.9, 0.5, 0.5};
    double time = 0;
    double rotSpeed;
    float posX, posY, radius, angle;
    unsigned int VAO, VBO;
    std::vector<vertex> vertices;
    static Shader *shader;
    Terminal *terminal;
    bool active = false;
    CylinderLock(int segments, double rotSpeed, float radius, Terminal *terminal, int *width, int *height);
    int update(GLFWwindow* window, KeyState *keyState, double deltaTime);
    void rotateLock(double deltaTime);
    void updateVertices();
    void render();
    int processInput(GLFWwindow *window, KeyState *keyState, double deltaTime);
};



#endif //CYLINDERLOCK_H
