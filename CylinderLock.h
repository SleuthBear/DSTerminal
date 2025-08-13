//
// Created by Dylan Beaumont on 12/8/2025.
//

#ifndef CYLINDERLOCK_H
#define CYLINDERLOCK_H
#include "Shader.h"
#include <glfw3.h>
#include <vector>

#include "KeyState.h"

#define MAX_SEGMENTS 64

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
    CylinderLock(int segments, double rotSpeed, float posX, float posY, float radius);
    void rotateLock(double deltaTime);
    void updateVertices();
    void render(Shader &shader);
    void processInput(GLFWwindow *window, KeyState *keyState, double deltaTime);
};



#endif //CYLINDERLOCK_H
