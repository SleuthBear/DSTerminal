//
// Created by Dylan Beaumont on 21/8/2025.
//

#ifndef TITLESCREEN_H
#define TITLESCREEN_H
#include "../utils/Shader.h"
#include <glfw3.h>
#include <glm/vec2.hpp>

/* So I realllly hate this implementation. It requires inheritance, and virtual functions and looping and
 * the base class to contain a bunch of shit, and it's bad. I'll replace it when I get smarter. */

struct Button {
    glm::vec2 topLeft;
    glm::vec2 botRight;
    glm::vec2 texTL;
    glm::vec2 texBR;
    std::function<void()> press;
};

struct Visual {
    glm::vec2 topLeft;
    glm::vec2 botRight;
    glm::vec2 texTL;
    glm::vec2 texBR;
};

struct Layout {
    Button *buttons;
    Visual *visual;
};

class UI {
public:
    unsigned int VBO, VAO;
    int *scrWidth;
    int *scrHeight;
    static Shader *shader;
    bool active = false;
    bool exited = false;
    bool shouldClose = false;
    std::vector<Button> buttons;
    std::vector<Visual> visuals;
    static int state;
    static unsigned int tex;
    // default constructor
    UI(int *width, int*height);
    int update(GLFWwindow *window, double deltaTime);
    void cleanup();
    void renderButtons();
    void renderVisuals();
    // Ok look. I know this sucks. But it also is kinda clean, so I don't want to get rid of it.
    virtual unsigned int getTex() {return 0;};
};

#endif //TITLESCREEN_H
