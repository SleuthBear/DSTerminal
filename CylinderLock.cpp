//
// Created by Dylan Beaumont on 12/8/2025.
//

#include "CylinderLock.h"

#include <glm/gtc/type_ptr.hpp>


#define PI 3.14159265
CylinderLock::CylinderLock(int segments, double rotSpeed, float posX, float posY, float radius) {
    this->segments = segments;
    this->rotSpeed = rotSpeed;
    this->angle = 0;
    this->radius = radius;
    this->posX = posX;
    this->posY = posY;
    this->updateVertices();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vertex), nullptr, GL_DYNAMIC_DRAW); // be sure to use glBufferSubData and not glBufferData

}

void CylinderLock::updateVertices() {
    vertices.clear();
    float segAngle = 2.0*PI / (float)segments;
    for (int i = 0; i < segments; i++) {
        float x;
        float y;
        if (sunkSegments[i]) {
            x = posX;
            y = posY;
        } else {
            x = posX + radius*cos(angle+segAngle/2)/3.0;
            y = posY + radius*sin(angle+segAngle/2)/3.0;
        }
        // counter-clockwise initialisation;
        vertices.push_back({x, y, 0});
        vertices.push_back({x+radius*std::cos(angle), y+radius*std::sin(angle), 0});
        angle += segAngle;
        vertices.push_back({x+radius*std::cos(angle),y+radius*std::sin(angle), 0});
    }
}

//todo shader rotation matrix
void CylinderLock::rotateLock(double deltaTime) {
    this->angle += rotSpeed*deltaTime;
    updateVertices();
}

void CylinderLock::render(Shader &shader) {
    // todo let the class have the shader
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "color"), color.x, color.y, color.z);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size()*sizeof(vertex), vertices.data());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

void CylinderLock::processInput(GLFWwindow *window, KeyState *keyState, double deltaTime) {
    float upAngle = fmod(angle-PI/2, PI*2);
    printf("%f\n", upAngle);
    int segment = segments - ((float)upAngle / (2*PI / (float)segments));
    printf("%d\n", segment);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (keyState->space > 5*keyState->interval) {
            sunkSegments[segment] = 1;
            keyState->space = 0;
        }
    }
    for (int i = 0; i < segments; i++) {
        if (sunkSegments[i] == 0) {
            return;
        }
    }
    color = {0.5, 0.9, 0.5};
}

