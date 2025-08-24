//
// Created by Dylan Beaumont on 21/8/2025.
//

#include "UI.h"

#include <cstdio>
#include <glfw3.h>
#include <iostream>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/type_ptr.inl"

// these don't do anything, we are just unbind the previous callbacks
void titleScreenCharCallback(GLFWwindow *window, unsigned int codepoint) {}
void titleScreenScrollCallback(GLFWwindow *window, double xOffset, double yOffset) {}
void titleScreenMouseButtonCallback(GLFWwindow *window, int action, int button, int mods);
Shader *UI::shader = nullptr;

UI::UI(int *width, int *height) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    scrWidth = width;
    scrHeight = height;
}

int UI::update(GLFWwindow *window, double deltaTime) {
    if (!active) {
        active = true;
        glfwSetWindowUserPointer(window, this);
        glfwSetCharCallback(window, titleScreenCharCallback);
        glfwSetScrollCallback(window, reinterpret_cast<GLFWscrollfun>(titleScreenCharCallback));
        glfwSetMouseButtonCallback(window, titleScreenMouseButtonCallback);
    }
    renderButtons();
    renderVisuals();
    if (exited) {
        return 1;
    }
    if (shouldClose) {
        glfwSetWindowShouldClose(window, true);
    }
    return 0;
}

void UI::renderVisuals() {
    std::vector<float> vertices;
    for (Visual visual : visuals) {
        float lx = visual.topLeft.x**scrWidth;
        float rx = visual.botRight.x**scrWidth;
        float uy = *scrHeight - visual.topLeft.y**scrHeight;
        float dy = *scrHeight - visual.botRight.y**scrHeight;
        float tlx = visual.texTL.x;
        float trx = visual.texBR.x;
        float tuy = visual.texTL.y;
        float tdy = visual.texBR.y;
        vertices.emplace_back(lx), vertices.emplace_back(uy), vertices.emplace_back(tlx), vertices.emplace_back(tuy),
        vertices.emplace_back(lx), vertices.emplace_back(dy), vertices.emplace_back(tlx), vertices.emplace_back(tdy),
        vertices.emplace_back(rx), vertices.emplace_back(uy), vertices.emplace_back(trx), vertices.emplace_back(tuy),

        vertices.emplace_back(rx), vertices.emplace_back(uy), vertices.emplace_back(trx), vertices.emplace_back(tuy),
        vertices.emplace_back(lx), vertices.emplace_back(dy), vertices.emplace_back(tlx), vertices.emplace_back(tdy),
        vertices.emplace_back(rx), vertices.emplace_back(dy), vertices.emplace_back(trx), vertices.emplace_back(tdy);
    }
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(*scrWidth), 0.0f, static_cast<float>(*scrHeight));
    shader->use();
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader->ID, "spriteColor"), 1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, getTex());
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()/4);
}

void UI::renderButtons() {
    // TODO only re-render on state change.
    // For some strange reason cursor position and drawing position are reversed so I am doing that just for this function
    std::vector<float> vertices;
    for(Button button : buttons) {
        float lx = button.topLeft.x**scrWidth;
        float rx = button.botRight.x**scrWidth;
        float uy = *scrHeight - button.topLeft.y**scrHeight;
        float dy = *scrHeight - button.botRight.y**scrHeight;
        vertices.emplace_back(lx), vertices.emplace_back(uy), vertices.emplace_back(button.texTL.x), vertices.emplace_back(button.texTL.y),
        vertices.emplace_back(lx), vertices.emplace_back(dy), vertices.emplace_back(button.texTL.x), vertices.emplace_back(button.texBR.y),
        vertices.emplace_back(rx), vertices.emplace_back(uy), vertices.emplace_back(button.texBR.x), vertices.emplace_back(button.texTL.y),

        vertices.emplace_back(rx), vertices.emplace_back(uy), vertices.emplace_back(button.texBR.x), vertices.emplace_back(button.texTL.y),
        vertices.emplace_back(lx), vertices.emplace_back(dy), vertices.emplace_back(button.texTL.x), vertices.emplace_back(button.texBR.y),
        vertices.emplace_back(rx), vertices.emplace_back(dy), vertices.emplace_back(button.texBR.x), vertices.emplace_back(button.texBR.y);
    }

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(*scrWidth), 0.0f, static_cast<float>(*scrHeight));
    shader->use();
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader->ID, "spriteColor"), 1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, getTex());
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()/4);
}


void titleScreenMouseButtonCallback(GLFWwindow *window, int action, int button, int mods) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        UI *screen = static_cast<UI*>(glfwGetWindowUserPointer(window));
        for (Button button : screen->buttons) {
            const float lx = button.topLeft.x**screen->scrWidth;
            const float rx = button.botRight.x**screen->scrWidth;
            const float uy = button.topLeft.y**screen->scrHeight;
            const float dy = button.botRight.y**screen->scrHeight;
            if (xPos > lx && xPos < rx && yPos > uy && yPos < dy) {
                button.press();
            }
        }
    }
}

