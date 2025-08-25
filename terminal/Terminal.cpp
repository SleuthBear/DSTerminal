//
// Created by Dylan Beaumont on 4/8/2025.
//

#include "Terminal.h"
#include <freetype/freetype.h>
#include <STB_IMAGE/stb_image_write.h>

#include "../locks/CylinderLock.h"
#include "../include/STB_IMAGE/stb_image.h"
#include "STB_IMAGE/stb_image.h"

void terminalScrollCallback(GLFWwindow *window, double xOffset, double yOffset);
void terminalCharCallback(GLFWwindow *window, unsigned int codepoint);
Shader* Terminal::shader = nullptr;  // Definition

Terminal::Terminal(FileNode root, FileNode node, std::function<void(GameLayer)> pushToStack, int *width, int *height) {
    getDialogue("../dialogue.json");
    this->width = width;
    this->height = height;
    this->root = &root;
    this->node = &node;
    this->pushToStack = pushToStack;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    lines[0].text = "";
    lines[1].text = "";
    lines[1].colour[0] = 0; lines[1].colour[1] = 0.9f; lines[1].colour[2] = 0;
    lines[1].side = DS_USER;
    end = 0;
    start = 1;
    window = 1;
    for (int i = 0; i < MAX_COMMAND_LOOKBACK; i++) {
        commands[i] = {"", GREEN, DS_USER};
    }
    createBitMap(std::string("../resources/ModernDOS.ttf").c_str(), &atlasTex, characters);
    input = "";
}

int Terminal::update(GLFWwindow *window, KeyState *keyState, double deltaTime) {
    termTime += deltaTime;
    if (!active) {
        glfwSetWindowUserPointer(window, this);
        glfwSetCharCallback(window, terminalCharCallback);
        glfwSetScrollCallback(window, reinterpret_cast<GLFWscrollfun>(terminalScrollCallback));
        glfwSetMouseButtonCallback(window, nullptr);
        active = true;
    }
    effects.update(deltaTime);
    processInput(window, keyState, deltaTime);
    renderBuffer(*shader, {5.0f, 5.0f}, (float)*width-15.0f, (float)*height-15.0f);
    imp->update(window, deltaTime);
    shader->use();
    return 0;
}

void Terminal::renderText(Shader &shader, std::string text, float xInitial, float y, std::vector<int> lineWraps, float width, float lineHeight, float scale, vec3 color) {
    // activate corresponding render state
    shader.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    if (effects.colorTimer > 0) {
        glUniform3f(glGetUniformLocation(shader.ID, "textColor"), effects.color.x, effects.color.y, effects.color.z);
    } else {
        glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color[0], color[1], color[2]);
    }
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
            if (effects.shaking>0) {
                yPos += 0.2*lineHeight*std::sinf(xPos+yPos+termTime*5);
            }

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

std::vector<int> Terminal::getLineWraps(std::string_view text, float x, float width, float scale) {
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

// Render the terminal with top-left corner pos.
void Terminal::renderBuffer(Shader shader, glm::vec2 pos, float width, float height) {
    shader.use();
    if (window == -1) return;
    // Subtract line spacing from height to space the bottom line.
    int nLines = ceil((height - LINE_SPACING) / lineHeight) + 1;
    int cHeight = characters['l'].Size[1]*1.2;
    float charHeight = lineHeight - LINE_SPACING;
    float charScale = charHeight / (cHeight);
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
        std::vector<int> wraps = getLineWraps(toDisplay, 0, width, charScale);
        printedLines += wraps.size();
        // One line spacing to pad the input line
        float lineY =  LINE_SPACING + (lineHeight) * (float)(printedLines-1);
        renderText(shader, toDisplay, pos.x, lineY, wraps, width, lineHeight, charScale, line.colour);
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

    if (dialogue.contains(node->fileRef)) {
        for (const Dialogue& option : dialogue[node->fileRef]) {
            // todo add matching if the text just exists in the string
            if (option.input == text && text != "") {
                imp->text = option.response;
                imp->timeToSpeak = 5;
            }
        }
    }
    std::vector<std::string_view> args;
    int argIdx = text.find_first_not_of(' ');
    while (argIdx != -1) {
        text = text.substr(argIdx);
        int argEnd = text.find_first_of(' ', argIdx);
        int nextNewLine = text.find_first_of('\n', argIdx);
        if (argEnd == -1 || (nextNewLine != -1 && nextNewLine < argEnd)) {
           argEnd = nextNewLine;
        }
        if (argEnd == -1) {
            argEnd = text.length();
        }
        args.emplace_back(text.substr(0, argEnd));
        argIdx = text.find_first_not_of(' ', argEnd);
    }
    if (args.size() == 0) {
        return;
    }

    if (args[0] == "ls") {
        if (args.size() == 1) {
            ls("");
        } else {
            ls(args[1]);
        }
        return;
    }
    if (args[0] == "cd") {
        if (args.size() == 1) {
            addLine({"Invalid file target", RED, DS_SYS});
            return;
        }
        cd(args[1]);
    }
    if (args[0] == "cat") {
        if (args.size() == 1) {
            addLine({"Invalid file target", RED, DS_SYS});
            return;
        }
        cat(args[1]);
    }
    if (args[0] == "help") {
        imp->timeToSpeak = 5;
        imp->charsToPrint = 0;
        imp->text = "Help? Do I look like a butler?!";
    }
    if (args[0] == "fuck") {
        imp->timeToSpeak = 5;
        imp->charsToPrint = 0;
        imp->text = "Watch your language you little shit.";
    }
    if (args[0] == "shake") {
        effects.shaking = 5.0f;
    }
    if (args[0] == "RED") {
        effects.colorTimer = 5.0f;
        effects.color = RED;
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
    // if (pos->name.find(".lock") != -1) {
    //     node = pos;
    //     CylinderLock *lock = new CylinderLock{10, 2, 100, this, width, height};
    //     pushToStack({
    //         [lock](GLFWwindow* _window, KeyState *_keyState, double _deltaTime){return lock->update(_window, _keyState, _deltaTime);},
    //             [lock]() {delete lock;}
    //     });
    //     active = false;
    // }
    node = pos;
    if (dialogue.contains(node->fileRef)) {
        std::string_view toSpeak = dialogue[node->fileRef][0].response;
        if (toSpeak != "") {
            imp->text = toSpeak;
            imp->timeToSpeak = 5.0f;
        }
    }
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        MenuScreen *menu = new MenuScreen(width, height);
        pushToStack(
            {[menu](GLFWwindow *_window, KeyState *keystate, double deltaTime){return menu->update(_window, deltaTime);},
            [menu](){return;}}
            );
        active = false;
    }
}

void terminalCharCallback(GLFWwindow *window, unsigned int codepoint) {
    Terminal *terminal = static_cast<Terminal*>(glfwGetWindowUserPointer(window));
    int start = static_cast<int>(terminal->start);
    int cursor = terminal->cursor;
    terminal->lines[start].text.insert(cursor, 1, static_cast<char>(codepoint));
    terminal->viewHeight = 0;
    terminal->window = terminal->start;
    terminal->cursor++;
}

void terminalScrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
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

void Terminal::getDialogue(std::string path) {
    std::ifstream f(path);
    json data = json::parse(f);
    for (json d : data) {
        std::cout << d["file_ref"] << std::endl;
        std::vector<Dialogue> options;
        for (json option : d["options"]) {
            options.push_back({option["input"], option["response"]});
        }
        dialogue[d["file_ref"]] = options;
    }
}