//
// Created by Dylan Beaumont on 18/8/2025.
//

#include "../imp/Imp.h"
#include <glm/gtc/type_ptr.hpp>

Imp::Imp(int* scrWidth, int* scrHeight) {
    this->text = "Welcome to the Daemon Shell. I'm Imp, a personal assistant invented by Dr. Hollenfeuer. Type help for more info!";
    this->timeToSpeak = 15.0f;
    this->scrWidth = scrWidth;
    this->scrHeight = scrHeight;
    TextureLoader texLoader;
    tex = texLoader.loadTexture("../resources/mon3_sprite_text.png");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 60*sizeof(float), nullptr, GL_DYNAMIC_DRAW); // be sure to use glBufferSubData and not glBufferData
    createBitMap("../resources/ModernDOS.ttf", &atlasTex, characters);
}

void Imp::update(GLFWwindow *window, double deltaTime) {
    time += deltaTime;
    glm::vec2 pos = {*scrWidth * 0.66, *scrHeight*0.66};
    // float scale = std::min(*scrWidth, *scrHeight)*0.25;
    float scale = 200.0f;
    glm::vec2 texCoord = frames[(int)fmod(time*5, 5)];
    // Logic for making the textbox big enough (irrelevant if not speaking)
    float charScale = 0.5;
    // todo pick a line height
    float lineHeight = scale/8;
    std::vector<int> wraps = getLineWraps(text, 0, 1.3f*scale, charScale);
    float boxDepth = (wraps.size())*lineHeight+10;
    float vertices[] = {
        pos.x,         pos.y+scale,     texCoord.x, 0.0,
        pos.x,         pos.y,           texCoord.x, 0.25,
        pos.x + scale, pos.y,           texCoord.y, 0.25,

        pos.x,         pos.y + scale,   texCoord.x, 0.0,
        pos.x + scale, pos.y,           texCoord.y, 0.25,
        pos.x + scale, pos.y + scale,   texCoord.y, 0.0,


        // Text box rendering                   Sample a translucent white
        pos.x-1.5f*scale,   pos.y+0.8f*scale,   0.9, 0.1,
        pos.x-1.5f*scale,   pos.y+0.8f*scale-boxDepth,     0.9, 0.1,
        pos.x,              pos.y+0.8f*scale-boxDepth,     0.9, 0.1,

        pos.x-1.5f*scale,   pos.y+0.8f*scale,   0.9, 0.1,
        pos.x,              pos.y+0.8f*scale-boxDepth,     0.9, 0.1,
        pos.x,              pos.y+0.8f*scale, 0.9, 0.1,

        pos.x+0.2f*scale,   pos.y+0.5f*scale,   0.9, 0.1,
        pos.x,              pos.y+0.6f*scale,   0.9, 0.1,
        pos.x,              pos.y+0.4f*scale,   0.9, 0.1,

    };

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(*scrWidth), 0.0f, static_cast<float>(*scrHeight));
    shader->use();
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glUniform3f(glGetUniformLocation(shader->ID, "spriteColor"), color.x, color.y, color.z);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    if (timeToSpeak > 0) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, 60*sizeof(float), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 15);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, 24*sizeof(float), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    // glFlush();

    /// Now render the text
    if (timeToSpeak > 0) {
        if (charsToPrint < text.length()) {
            int oldCount = charsToPrint;
            charsToPrint += deltaTime*printSpeed;
            if (oldCount < (int)charsToPrint) {
                soundManager->play();
            }
        }
        int cHeight = characters['l'].Size[1]*1.5;
        timeToSpeak -= deltaTime;
        float charHeight = 0.90f*lineHeight;
        float charScale = charHeight / (cHeight);
        std::vector<int> wraps = getLineWraps(text, 0, 1.3f*scale, charScale);
        glUniformMatrix4fv(glGetUniformLocation(textShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        if ((int)charsToPrint > 0) {
            vec3 color = WHITE;
            renderText(*textShader, text, pos.x-1.4f*scale, pos.y+0.8f*scale-lineHeight, wraps, 1.3f*scale, lineHeight, charScale, color);
        }
    } else {
        charsToPrint = 0;
    }
}

void Imp::renderText(Shader &shader, std::string text, float xInitial, float y, std::vector<int> lineWraps, float width, float lineHeight, float scale, vec3 color) {
    // activate corresponding render state
    shader.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color[0], color[1], color[1]);
    glActiveTexture(GL_TEXTURE0);
    // iterate through all characters
    float x = xInitial;
    std::string::const_iterator c;
    int i = 0;
    std::vector<float> allVertices;
    // Bind the atlas texture which has all characters in it.
    int at = 0;
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    for (int nChars : lineWraps) {
        for (int j = 0; j < nChars; ++j) {
            char c = text[at++];
            Character ch = characters[c];
            float xPos = x + ch.Bearing[0] * scale;
            float yPos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

            float w = ch.Size[0] * scale;
            float h = ch.Size[1] * scale;
            // update VBO for each character
            float vertices[] = {
                 xPos,     yPos + h,   ch.uv[0], ch.uv[2],
                 xPos,     yPos,       ch.uv[0], ch.uv[3],
                 xPos + w, yPos,       ch.uv[1], ch.uv[3],

                 xPos,     yPos + h,   ch.uv[0], ch.uv[2],
                 xPos + w, yPos,       ch.uv[1], ch.uv[3],
                 xPos + w, yPos + h,   ch.uv[1], ch.uv[2],
            };
            // todo clean up
            for (float f: vertices) {
                allVertices.push_back(f);
            }
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            i++;
        }
        x = xInitial;
        y -= lineHeight;
    }
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    // update content of VBO memory
    int toPrint = std::min((int)allVertices.size(), (int)charsToPrint*24);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size()*sizeof(float), allVertices.data(), GL_STATIC_DRAW); // be sure to use glBufferSubData and not glBufferData
    // render quads
    glDrawArrays(GL_TRIANGLES, 0, toPrint / 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

//todo line wraps returns the character counts for each line
std::vector<int> Imp::getLineWraps(std::string_view text, float x, float width, float scale) {
    int wraps = 1;
    std::string_view::const_iterator c;
    std::vector<int> lineWraps;
    int lineEnd = -1;
    int lineStart = 0;
    int i = 0;
    for (i = 0; i < text.length(); ++i) {
        Character ch = characters[text[i]];
        if (text[i] == ' ' && i > 2) {
            lineEnd = i;
        }
        float xPos = x + ch.Bearing[0] * scale;
        if (xPos >= width) {
            if (lineEnd == -1) {
                lineWraps.push_back(i-lineStart);
                lineStart = i;
            } else {
                lineWraps.push_back(lineEnd-lineStart+1);
                lineStart = lineEnd+1;
            }
            lineEnd = -1;
            wraps += 1;
            x = 0;
            for (int j = lineStart; j < i; j++) {
                x += (float)(characters[text[j]].Advance >> 6) * scale;
            }
        }
        x += (float)(ch.Advance >> 6) * scale;
    }
    lineWraps.push_back(i - lineStart);
    return lineWraps;
}
