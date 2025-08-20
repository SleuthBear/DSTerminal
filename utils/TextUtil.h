//
// Created by Dylan Beaumont on 18/8/2025.
//

#ifndef TEXTUTIL_H
#define TEXTUTIL_H
#include <glad.h>
#include <map>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>



static const glm::vec3 GREEN = {0, 0.9, 0};
static const glm::vec3 BLUE = {0.4, 0.4, 0.9};
static const glm::vec3 RED = {0.9, 0.4, 0.4};
static const glm::vec3 WHITE = {0.8, 0.8, 0.8};

struct Character {
 glm::ivec2   Size;      // Size of glyph
 glm::ivec2   Bearing;// Offset from baseline to left/top of glyph
 unsigned int Advance;   // Horizontal offset to advance to next glyph
 glm::vec4    uv;
};

unsigned int createBitMap(std::string path, unsigned int *atlasTex, std::map<GLchar, Character> &characters);



#endif //TEXTUTIL_H
