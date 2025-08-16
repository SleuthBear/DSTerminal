//
// Created by Dylan Beaumont on 4/8/2025.
//

#include "Terminal.h"
#include <freetype/freetype.h>
#include <STB_IMAGE/stb_image_write.h>

#include "CylinderLock.h"
#include "../include/STB_IMAGE/stb_image.h"
#include "STB_IMAGE/stb_image.h"

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
void charCallback(GLFWwindow *window, unsigned int codepoint);
Shader* Terminal::shader = nullptr;  // Definition

Terminal::Terminal(FileNode root, FileNode node, std::function<void(GameLayer)> pushToStack, int *width, int *height) {
    this->width = width;
    this->height = height;
    this->root = &root;
    this->node = &node;
    // What do you mean this isn't readable??
    this->pushToStack = pushToStack;
    // initialize vao and vbo
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    lines[0].text = "";
    lines[1].text = "";
    lines[1].colour = GREEN;
    lines[1].side = DS_USER;
    end = 0;
    start = 1;
    window = 1;
    for (int i = 0; i < MAX_COMMAND_LOOKBACK; i++) {
        commands[i] = {"", GREEN, DS_USER};
    }
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        exit(1);
    }

	// find path to font
    std::string font_name = "../resources/ModernDOS.ttf"; // Third-party fonts
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        exit(1);
    }

	// load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        exit(1);
    }
    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int maxWidth = 0;
    int maxHeight = 0;
    // load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->bitmap.buffer,
            static_cast<unsigned int>(face->glyph->advance.x),
        };
        maxWidth = std::max(character.Size.x, maxWidth);
        maxHeight = std::max(character.Size.y, maxHeight);
        characters.insert(std::pair<char, Character>(c, character));
    }
    lHeight = static_cast<float>(characters['l'].Size.y);
    glBindTexture(GL_TEXTURE_2D, 0);
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // just a very long texture
    unsigned char *atlas = new unsigned char[characters.size()*maxWidth*maxHeight];
    int atlasRow = maxWidth*characters.size();
    int count = 0;
    for(std::pair<char, Character> pair : characters) {
        Character c = pair.second;
        glBindTexture(GL_TEXTURE_2D, c.TextureID);
        unsigned char* pixels = new unsigned char[c.Size.x * c.Size.y*4]{}; // 4 = RGBA channels
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        for (int i = 0; i < c.Size.y; ++i) {
            for (int j = 0; j < c.Size.x; ++j) {
                // copy pixel data over
                memcpy(atlas + (i*atlasRow+count*maxWidth+j)*4*sizeof(unsigned char), pixels+(i*c.Size.x+j)*4*sizeof(unsigned char), 4);
            }
        }
        // store the x coordinates. Since it is a strip; y coordinates will always be 0 - 1
        float cPos = (float)count / (float)characters.size();
        // x-start, x-end, y-start, y-end
        uvMap[pair.first] = {cPos, cPos + (float)c.Size.x / (float)atlasRow, 0, (float)c.Size.y / (float)maxHeight};
        count++;
    }
    glGenTextures(1, &atlasTex);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasRow, maxHeight,
             0, GL_RGBA, GL_UNSIGNED_BYTE, atlas);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] atlas;
    input = "";
}

int Terminal::update(GLFWwindow *window, KeyState *keyState, double deltaTime) {
    // todo fix magic numbers
    if (!active) {
        glfwSetWindowUserPointer(window, this);
        glfwSetCharCallback(window, charCallback);
        glfwSetScrollCallback(window, reinterpret_cast<GLFWscrollfun>(scrollCallback));
        active = true;
    }
    processInput(window, keyState, deltaTime);
    renderBuffer(*shader, {5.0f, 5.0f}, (float)*width-15.0f, (float)*height-15.0f);
    return 0;
}

// todo bundle all the data together and do a single draw call.
void Terminal::renderText(Shader &shader, std::string text, float xInitial, float y, float width, float lineHeight, float scale, glm::vec3 color) {
    // activate corresponding render state
    shader.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    // iterate through all characters
    float x = xInitial;
    std::string::const_iterator c;
    int i = 0;
    std::vector<float> allVertices;
    // Bind the atlas texture which has all characters in it.
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        // If we have wrapped, move down a line then keep printing.
        if (xpos >= width) {
            y -= lineHeight;
            x = xInitial;
            xpos = x + ch.Bearing.x * scale;
        }
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[] = {
             xpos,     ypos + h,   uvMap[*c].x, uvMap[*c].z,
             xpos,     ypos,       uvMap[*c].x, uvMap[*c].w,
             xpos + w, ypos,       uvMap[*c].y, uvMap[*c].w,

             xpos,     ypos + h,   uvMap[*c].x, uvMap[*c].z,
             xpos + w, ypos,       uvMap[*c].y, uvMap[*c].w,
             xpos + w, ypos + h,   uvMap[*c].y, uvMap[*c].z,
        };
        // todo clean up
        for (float f: vertices) {
            allVertices.push_back(f);
        }
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        i++;
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

int Terminal::getLineWraps(std::string_view text, float x, float width, float scale) {
    int wraps = 1;
    std::string_view::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];
        float xpos = x + ch.Bearing.x * scale;
        // Check for wrapping
        if (xpos >= width) {
            wraps += 1;
            x = 0;
        }
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    return wraps;
}

// Render the terminal with top-left corner pos.
// todo use string views
void Terminal::renderBuffer(Shader shader, glm::vec2 pos, float width, float height) {
    shader.use();
    if (window == -1) return;
    // Subtract line spacing from height to space the bottom line.
    int nLines = ceil((height - LINE_SPACING) / lineHeight) + 1;
    float charHeight = lineHeight - LINE_SPACING;
    float charScale = charHeight / lHeight;
    // We render nLines - 1. Because the last line is the one currently being typed.
    int printedLines = 0;
    // renderText(shader, "> " + input, pos.x, pos.y + (float)(printedLines-1)*lineHeight, width, lineHeight, charScale, GREEN);
    for (int i = 0; printedLines < (nLines); i++) {
        // Upon reaching the start of the buffer, simply wrap around.
        if (i == -1) {
            i = MAX_LINES-1;
        }

        Line line = lines[window - i];
        std::string toDisplay;
        if (line.side == DS_USER) {
            toDisplay = "> " + line.text;
        } else {
            toDisplay = line.text;
        }
        if (window-i == start) {
            if (toDisplay.length() < cursor+2) {
                toDisplay += "|";
            } else {
                toDisplay.replace(cursor+2, 1, "|");
            }
        }
        printedLines += getLineWraps(toDisplay, 0, width, charScale); //renderTextWithWrap(shader, "> " + input, pos.x, pos.y, width, lineHeight, charScale, GREEN);
        // One line spacing to pad the input line
        float lineY =  LINE_SPACING + (lineHeight) * (float)(printedLines-1);

        renderText(shader, toDisplay, pos.x, lineY, width, lineHeight, charScale, line.colour);
        // can't skip past the end of the buffer.
        if (window-i == end) {
            break;
        }
    }
}

void Terminal::addLine(const Line& line) {
    // if end != start then buffer hasn't been filled.
    if (end != start) {
        if (start == MAX_LINES - 1) {
            start = 0;
            lines[start] = line;
        } else {
            lines[++start] = line;
        }
    } else {
        lines[++start] = line;
        end++;
    }
    window = start;
}

void Terminal::stepBack() {
    node = node->parent;
}

// Todo figure out how to do regex. (roll your own regex? that's gonna suck)
// Maybe just delimit args by spaces? :)
void Terminal::readCommand() {
   // Handle the lookback buffer
    if (atCommand == MAX_COMMAND_LOOKBACK-1) {
        atCommand = 0;
    } else {
        atCommand++;
    }
    commands[atCommand] = {std::string(lines[start].text), GREEN, DS_USER};
    viewingCommand = atCommand;

    std::string_view text = lines[start].text;
    if (text.substr(0, 2) == "ls") {
        if (text.length() < 3) {
            ls("");
        } else {
            ls(text.substr(3, text.length()-3));
        }
        return;
    }
    if (text.substr(0,2) == "cd") {
        if (text.length() < 3) {
            addLine({"Invalid file target", RED, DS_SYS});
            return;
        }
        cd(text.substr(3, text.length()-3));
    }
    if (text.substr(0,3) == "cat") {
        if (text.length() < 4) {
            addLine({"Invalid file target", RED, DS_SYS});
            return;
        }
        cat(text.substr(3, text.length()-3));
    }
}

void Terminal::ls(std::string_view path) {
    std::string response;
    // Recurse through the path and try exit out if you fail to get to the end
    FileNode *pos = node;
    pos = followPath(pos, path);
    if (pos == nullptr) {
        addLine({"Invalid path.", RED, DS_SYS});
        return;
    }
    if (node->name != pos->name && pos->name.find(".lock") != -1) {
        addLine({"Must unlock " + pos->name, RED, DS_SYS});
        return;
    }
    for (int i=0; i < pos->nChildren; i++) {
        response += pos->children[i]->name + "  ";
    }
    addLine({response, BLUE, DS_SYS});
}

void Terminal::cd(const std::string_view path) {
    FileNode *pos = node;
    pos = followPath(pos, path);
    if (pos == nullptr) {
        addLine({"Invalid path.", RED, DS_SYS});
        return;
    }
    if (pos->name.find(".lock") != -1) {
        node = pos;
        CylinderLock *lock = new CylinderLock{10, 2, 100, this, width, height};
        pushToStack({
            [lock](GLFWwindow* _window, KeyState *_keyState, double _deltaTime){return lock->update(_window, _keyState, _deltaTime);},
                [lock]() {delete lock;}
        });
        active = false;
    }
    node = pos;
}

void Terminal::cat(const std::string_view path) {
    std::string response;
    // Recurse through the path and try exit out if you fail to get to the end
    FileNode *pos = node;
    pos = followPath(pos, path);
    if (pos == nullptr) {
        addLine({"Invalid path.", RED, DS_SYS});
        return;
    }
    if (pos->type != DS_FILE) {
        addLine({"Not a file." + pos->name, RED, DS_SYS});
        return;
    }
    std::string fileRef = pos->fileRef;
    std::ifstream stream("../files/" + fileRef);
    if(!stream.is_open()) {
        addLine({"Unable to open the file.", RED, DS_SYS});
    }
    std::string line;
    while (std::getline(stream, line)) {
        addLine({line, WHITE, DS_SYS});
    }
}

void Terminal::processInput(GLFWwindow *window, KeyState* keyState, double deltaTime) {
    keyState->addDeltaTime(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        if (keyState->backspace > keyState->interval) {
            unsigned int start = this->start;
            std::string *input = &lines[start].text;
            if (!input->empty()) {
                input->erase(cursor-1, 1);
                cursor--;
            }
            keyState->backspace = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        if (keyState->enter > keyState->interval) {
            if (!lines[this->start].text.empty()) {
                readCommand();
                addLine({"", GREEN, DS_USER});
                viewHeight = 0;
                cursor = 0;
            }
            keyState->enter = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (keyState->tab > keyState->interval) {
            autoComplete();
            keyState->tab = 0;
        }

    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && cursor > 0) {
        if (keyState->left > keyState->interval) {
            cursor--;
            keyState->left = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && cursor < lines[this->start].text.length()) {
        if (keyState->right > keyState->interval) {
            cursor++;
            keyState->right = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (keyState->up > keyState->interval) {
            lines[start] = commands[viewingCommand];
            cursor = lines[start].text.length();
            if (viewingCommand == 0) {
                viewingCommand = MAX_COMMAND_LOOKBACK-1;
            } else {
                viewingCommand -= 1;
            }
            keyState->up = 0;
        }
    }
}

void charCallback(GLFWwindow *window, unsigned int codepoint) {
    Terminal *terminal = static_cast<Terminal*>(glfwGetWindowUserPointer(window));
    int start = static_cast<int>(terminal->start);
    int cursor = terminal->cursor;
    terminal->lines[start].text.insert(cursor, 1, static_cast<char>(codepoint));
    terminal->viewHeight = 0;
    terminal->window = terminal->start;
    terminal->cursor++;
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
    Terminal *terminal = static_cast<Terminal*>(glfwGetWindowUserPointer(window));
    float lineHeight = terminal->lineHeight;
    double yPos = terminal->viewHeight;
    int end = terminal->end;
    int start = terminal->start;

    // If there aren't enough lines to scroll at all, just return
    if(start > end && (start - end) < (double)*terminal->height/lineHeight) {
        return;
    }
    yPos += yOffset;
    yPos = std::max(0.0, yPos);
    int availableLines = MAX_LINES;
    if (start > end) {
        availableLines = start-end;
    }
    // Must always have a full screen of lines available
    yPos = std::min((double)availableLines*lineHeight - (double)*terminal->height, yPos);
    terminal->window = static_cast<unsigned int>(static_cast<double>(start) - yPos/lineHeight);
    terminal->viewHeight = yPos;

}

void Terminal::autoComplete() {
    int i = cursor;
    while (i >= 0 && lines[start].text[i] != ' ' ) i--;
    std::string toComplete = lines[start].text.substr(i+1, cursor-i);
    i = toComplete.length();
    while (i >= 0 && toComplete[i] != '/') i--;
    FileNode *pos = followPath(node, toComplete.substr(0, i+1));
    if (pos == nullptr) {
        return;
    }
    std::string_view toSearch = toComplete.substr(i+1, toComplete.length()-i);
    std::vector<FileNode*> children;
    for (int j = 0; j < pos->nChildren; j++) {
        if (pos->children[j]->name.starts_with(toSearch)) {
            children.push_back(pos->children[j]);
        }
    }
    if (children.empty()) return;
    std::string completion;
    bool found = false;
    if (children.size() > 1) {
        for (i = toSearch.length(); i < children[0]->name.length(); i++) {
            for (FileNode *child : children) {
                if (child->name[i] != children[0]->name[i]) {
                    found = true;
                    break;
                }
            }
            if (found) break;
            completion += children[0]->name[i];
        }
    } else {
        completion = children[0]->name.substr(toSearch.length());
    }
    lines[start].text.insert(cursor, completion);
    cursor += completion.length();
}

