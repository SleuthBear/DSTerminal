//
// Created by Dylan Beaumont on 18/8/2025.
//

#include "TextUtil.h"

#include <glad.h>
#include <iostream>
#include <map>
#include <freetype/freetype.h>
#include <glm/vec4.hpp>
#include <STB_IMAGE/stb_image_write.h>

unsigned int createBitMap(std::string path, unsigned int *atlasTex,
                          std::map<GLchar, Character> &characters) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        exit(1);
    }

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        exit(1);
    }
    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Get the layout for the atlas
    if (FT_Load_Char(face, 'W', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        exit(1);
    }
    int maxWidth = face->glyph->bitmap.width*2;
    int maxHeight = face->glyph->bitmap.rows*2;
    unsigned char *atlas = new unsigned char[128*maxWidth*maxHeight*4];
    int atlasRow = maxWidth*128;
    int count = 0;
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // store information on the character for later rendering
        Character ch = {
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x),
            {0, 0, 0, 0}
        };
        int pitch = face->glyph->bitmap.pitch;
        for (int i = 0; i < ch.Size.y; ++i) {
            for (int j = 0; j < ch.Size.x; ++j) {
                // Monochrome, store in the red channel
                atlas[(i*atlasRow+count*maxWidth+j)*4] = face->glyph->bitmap.buffer[(i*pitch)+j];
                atlas[(i*atlasRow+count*maxWidth+j)*4+1] = 0;
                atlas[(i*atlasRow+count*maxWidth+j)*4+2] = 0;
                atlas[(i*atlasRow+count*maxWidth+j)*4+3] = 255;
            }
        }
        float cPos = (float)count / 128.0f;
        // Update the UV map for the character.
        // x-start, x-end, y-start, y-end
        ch.uv = {cPos, cPos + (float)ch.Size.x / (float)atlasRow, 0, (float)ch.Size.y / (float)maxHeight};
        characters.insert(std::pair<char, Character>(c, ch));
        count++;
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    glGenTextures(1, atlasTex);
    glBindTexture(GL_TEXTURE_2D, *atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasRow, maxHeight,
             0, GL_RGBA, GL_UNSIGNED_BYTE, atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFlush();
    delete[] atlas;
    return 0;
}
