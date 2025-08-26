//
// Created by Dylan Beaumont on 26/8/2025.
//

#include "RiddleLock.h"

void riddleLockCharCallback(GLFWwindow* window, unsigned int codepoint);
std::map<std::string, bool> LOCKS_OPENED;
RiddleLock::RiddleLock(std::string fileRef, LockInfo info, int* width, int* height) {
    this->hint = info.hint;
    this->answer = info.answer;
    this->fuzzy = info.fuzzy;
    this->scrWidth = width;
    this->scrHeight = height;
    this->fileRef = fileRef;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

}

int RiddleLock::update(GLFWwindow *window, KeyState *keyState, double deltaTime) {
    if (!active) {
        glfwSetWindowUserPointer(window, this);
        glfwSetCharCallback(window, riddleLockCharCallback);
        glfwSetScrollCallback(window, nullptr);
        active = true;
    }
    //todo center the text properly
    int finished = processInput(window, keyState, deltaTime);
    render();
    return finished;
}


void riddleLockCharCallback(GLFWwindow* window, unsigned int codepoint) {
    RiddleLock *lock = static_cast<RiddleLock*>(glfwGetWindowUserPointer(window));
    lock->input.insert(lock->cursor, 1, static_cast<char>(codepoint));
    lock->cursor++;
}

int RiddleLock::processInput(GLFWwindow* window, KeyState *keyState, double deltaTime) {
    keyState->addDeltaTime(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        if (keyState->backspace > keyState->interval) {
            if (!input.empty()) {
                input.erase(cursor-1, 1);
                cursor--;
            }
            keyState->backspace = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        if (keyState->enter > keyState->interval) {
            if (!input.empty()) {
                if (input == answer) {
                    LOCKS_OPENED[fileRef] = true;
                    return UNLOCKED;
                }
                cursor = 0;
                input = "";
            }
            keyState->enter = 0;
        }
    }
    // if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && cursor > 0) {
    //     if (keyState->left > keyState->interval) {
    //         cursor--;
    //         keyState->left = 0;
    //     }
    // }
    // if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && cursor < input.length()) {
    //     if (keyState->right > keyState->interval) {
    //         cursor++;
    //         keyState->right = 0;
    //     }
    // }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        return LOCK_FAILED;
    }
    return 0;
}

void RiddleLock::render() {
    shader->use();
    int cHeight = characters['l'].Size[1]*1.2;
    float charHeight = *scrHeight/30.0f - LINE_SPACING;
    float charScale = charHeight / (cHeight);
    float lineHeight = 1.5*charHeight;
    float hintColor[] = {0.5f, 0.5f, 0.5f};
    float inputColor[] = {0.7f, 0.7f, 0.7f};
    // todo make this a C util instead of this crap
    std::vector<int> wraps = getLineWraps(hint, 0, *scrWidth*0.4, charScale);
    renderText(*shader, hint, *scrWidth*0.3, *scrHeight*0.6, wraps, *scrWidth*0.4, lineHeight, charScale, hintColor);
    wraps = getLineWraps(input, 0, *scrWidth*0.3, charScale);
    renderText(*shader, input, *scrWidth*0.3, *scrHeight*0.4, wraps, *scrWidth*0.4, lineHeight, charScale, inputColor);
}

std::vector<int> RiddleLock::getLineWraps(std::string_view text, float x, float width, float scale) {
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


void RiddleLock::renderText(Shader &shader, std::string text, float xInitial, float y, std::vector<int> lineWraps, float width, float lineHeight, float scale, vec3 color) {
    // activate corresponding render state
    shader.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color[0], color[1], color[2]);
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
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size()*sizeof(float), allVertices.data(), GL_STATIC_DRAW); // be sure to use glBufferSubData and not glBufferData
    // render quads
    glDrawArrays(GL_TRIANGLES, 0, allVertices.size() / 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}