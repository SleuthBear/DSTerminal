//
// Created by Dylan Beaumont on 18/8/2025.
//

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TEXTUTIL_H
#define TEXTUTIL_H
#include "../include/cglm/cglm.h"

// Macros used for interop with glm and cglm
#define GREEN {0, 0.9, 0}
#define BLUE {0.4, 0.4, 0.9}
#define RED {0.9, 0.4, 0.4}
#define WHITE {0.8, 0.8, 0.8}
#define LINE_SPACING 5.0f


// Structs //
typedef struct Character {
 ivec2   Size;      // Size of glyph
 ivec2   Bearing;// Offset from baseline to left/top of glyph
 unsigned int Advance;   // Horizontal offset to advance to next glyph
 vec4    uv;
} Character;

// typedef struct LineWraps {
//  int nWraps;
//  int* wraps;
// } LineWraps;

unsigned int createBitMap(const char* path, unsigned int* atlasTex, Character* characters);
// LineWraps getLineWraps(const char* text, float width, float scale, Character* characters);
// int renderText(const char* text, float xInitial, float y, float width, float lineHeight, float scale, Character* characters);

#endif //TEXTUTIL_H
#ifdef __cplusplus
}
#endif