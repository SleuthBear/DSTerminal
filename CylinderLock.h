//
// Created by Dylan Beaumont on 12/8/2025.
//

#ifndef CYLINDERLOCK_H
#define CYLINDERLOCK_H
#include "Shader.h"
#include <glfw3.h>
#include <vector>

#include "KeyState.h"
#include "Terminal.h"

#define MAX_SEGMENTS 64
#define UNLOCKED 2
#define LOCK_FAILED 3
struct vertex {
    float x, y, z;
};

class CylinderLock {
public:
    // Number of segments in the lock
    int segments;
    int sunkSegments[MAX_SEGMENTS]={};
    glm::vec3 color = {0.9, 0.5, 0.5};
    // Current time of the lock, used for rotation state.
    double time = 0;
    // rotational speed of the lock
    double rotSpeed;
    float posX, posY, radius, angle;
    unsigned int VAO, VBO;
    std::vector<vertex> vertices;
    static Shader *shader;
    // reference to the terminal it is blocking. Not sure if this is a good idea
    Terminal *terminal;
    bool active = false;
    CylinderLock(int segments, double rotSpeed, float posX, float posY, float radius, Terminal *terminal);
    int update(GLFWwindow* window, KeyState *keyState, double deltaTime);
    void rotateLock(double deltaTime);
    void updateVertices();
    void render();
    int processInput(GLFWwindow *window, KeyState *keyState, double deltaTime);
};



#endif //CYLINDERLOCK_H
