//
// Created by Dylan Beaumont on 18/8/2025.
//

#include "TextUtil.h"

#include <glad.h>
#include <freetype/freetype.h>
#include <STB_IMAGE/stb_image_write.h>

/* Creates a bitmap of characters in a provided font pack. This can be used as a texture with uv map which is added
 * to each character. */
unsigned int createBitMap(const char* path, unsigned int* atlasTex, Character* characters) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        printf("ERROR::FREETYPE: Could not init FreeType Library");
        exit(1);
    }

    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face)) {
        printf("ERROR::FREETYPE: Failed to load font");
        exit(1);
    }
    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Get the layout for the atlas
    if (FT_Load_Char(face, 'W', FT_LOAD_RENDER))
    {
        printf("ERROR::FREETYTPE: Failed to load Glyph");
        exit(1);
    }
    int maxWidth = face->glyph->bitmap.width*2;
    int maxHeight = face->glyph->bitmap.rows*2;
    unsigned char *atlas = (unsigned char*)malloc(128*maxWidth*maxHeight*4*sizeof(unsigned char));
    int atlasRow = maxWidth*128;
    int count = 0;
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printf("ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        }
        // store information on the character for later rendering
        struct Character ch = {
            {face->glyph->bitmap.width, face->glyph->bitmap.rows},
            {face->glyph->bitmap_left, face->glyph->bitmap_top},
            (unsigned int)(face->glyph->advance.x),
            {0, 0, 0, 0}
        };
        int pitch = face->glyph->bitmap.pitch;
        for (int i = 0; i < ch.Size[1]; ++i) {
            for (int j = 0; j < ch.Size[0]; ++j) {
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
        ch.uv[0] = cPos;
        ch.uv[1] = cPos + (float)ch.Size[0] / (float)atlasRow;
        ch.uv[2] = 0;
        ch.uv[3] = (float)ch.Size[1] / (float)maxHeight;

        characters[c] =  ch;
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
    free(atlas);
    return 0;
}

/* Calculates the line wraps required to print out the text in the specified area */
// LineWraps getLineWraps(const char* text, float width, float scale, Character* characters) {
//     int capacity = 10;
//     int *wraps = (int*)malloc(capacity*sizeof(int));
//     int lineEnd = -1;
//     int lineStart = 0;
//     int i = 0;
//     int nWraps = 0;
//     float x = 0;
//     while (text[i] != '\0') {
//         if (nWraps == capacity) {
//             capacity *= 2;
//             wraps = (int*)realloc(wraps, capacity*sizeof(int));
//             if (wraps == NULL) EXIT_REALLOC
//         }
//         Character ch = characters[text[i]];
//         if (text[i] == ' ' && i > 2) {
//             lineEnd = i;
//         }
//         float xPos = x + ch.Bearing[0] * scale;
//         if (xPos >= width) {
//             if (lineEnd == -1) {
//                 wraps[nWraps++] = i-lineStart;
//                 lineStart = i;
//             } else {
//                 wraps[nWraps++] = lineEnd-lineStart+1;
//                 lineStart = lineEnd+1;
//             }
//             lineEnd = -1;
//             x = 0;
//             for (int j = lineStart; j < i; j++) {
//                 x += (float)(characters[text[j]].Advance >> 6) * scale;
//             }
//         }
//         x += (float)(ch.Advance >> 6) * scale;
//         i++;
//     }
//     wraps[nWraps++] = i - lineStart;
//     return (LineWraps){nWraps, wraps};
// }