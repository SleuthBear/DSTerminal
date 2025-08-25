//
// Created by Dylan Beaumont on 18/8/2025.
//

#ifndef IMP_H
#define IMP_H
#include "../utils/TextureLoader.h"
#include <glfw3.h>
#include <glm/vec2.hpp>

#include "json.h"
#include "../utils/Shader.h"
#include "../utils/SoundManager.h"
#include "../utils/TextUtil.h"

class Imp {
public:
    SoundManager *soundManager;
    Shader *shader;
    Shader *textShader;
    GLuint atlasTex;
    std::string text = "";
    int *scrWidth;
    int *scrHeight;
    unsigned int VAO, VBO;
    double time;
    unsigned int tex;
    float timeToSpeak = 0;
    glm::vec3 color = RED;
    Character characters[128];
    double charsToPrint = 0;
    double printSpeed = 20;

    Imp(int* scrWidth, int* scrHeight);
    void update(GLFWwindow *window, double deltaTime);
    void renderText(Shader &shader, std::string text, float xInitial, float y, std::vector<int> lineWraps, float width, float lineHeight, float scale, vec3 color);
    std::vector<int> getLineWraps(std::string_view text, float x, float width, float scale);
    // x coordinates for rendering the texture
    glm::vec2 frames[5] = {
    {0.125, 0},
    {0.25, 0.125},
    {0.375,0.25},
    {0.500,0.375},
    {0.625,0.500}
    };
};



#endif //IMP_H
