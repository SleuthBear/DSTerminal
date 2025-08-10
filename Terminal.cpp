//
// Created by Dylan Beaumont on 4/8/2025.
//

#include "Terminal.h"

#include <freetype/freetype.h>

#include <utility>

Terminal::Terminal(FileNode root, FileNode node) {
    this->root = std::move(root);
    this->node = std::move(node);
    // initialize vao and vbo
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // initialize buffer
    lines[0].text = "123141";
    lines[1].text = "this is another line";
    lines[2].text = "oh look another line";
    lines[3].text = "";
    lines[3].colour = GREEN;
    lines[3].side = DS_USER;
    end = 0;
    start = 3;
    window = 3;
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
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    lHeight = static_cast<float>(characters['l'].Size.y);
    glBindTexture(GL_TEXTURE_2D, 0);
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    input = "";
}

void Terminal::renderText(Shader &shader, std::string text, float xInitial, float y, float width, float lineHeight, float scale, glm::vec3 color) {
    // activate corresponding render state
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    float x = xInitial;
    std::string::const_iterator c;
    int i = 0;
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
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // todo figure out how inefficient this is
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        i++;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int Terminal::getLineWraps(std::string text, float x, float width, float scale) {
    int wraps = 1;
    std::string::const_iterator c;
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
void Terminal::renderBuffer(Shader shader, glm::vec2 pos, float width, float height) {
    if (window == -1) return;
    // todo figure out scrolling to not show current line. Brilliant idea: just make the input the end of the buffer! problem solved.
    // Subtract line spacing from height to space the bottom line.
    int nLines = ceil((height - LINE_SPACING) / lineHeight) + 1;
    float charHeight = lineHeight - LINE_SPACING;
    float charScale = charHeight / lHeight;
    // We render nLines - 1. Because the last line is the one currently being typed.
    int printedLines = 0; // getLineWraps("> " + input, pos.x, width, charScale);
    // renderText(shader, "> " + input, pos.x, pos.y + (float)(printedLines-1)*lineHeight, width, lineHeight, charScale, GREEN);
    for (int i = 0; printedLines < (nLines); i++) {
        // Upon reaching the start of the buffer, simply wrap around.
        if (i == -1) {
            i = MAX_LINES-1;
        }

        Line line = lines[window - i];
        std::string toDisplay = "> " + line.text;
        if (window-i == start) {
            if (toDisplay.length() < cursor+2) {
                toDisplay += "|";
            } else {
                toDisplay.replace(cursor+2, 1, "|");
            }
        }
        printedLines += getLineWraps("> " + line.text, 0, width, charScale); //renderTextWithWrap(shader, "> " + input, pos.x, pos.y, width, lineHeight, charScale, GREEN);
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

// Todo figure out how to do regex. (roll your own regex? that's gonna suck)
void Terminal::readCommand() {
    std::string text = lines[start].text;
    if (text.substr(0, 2) == "ls") {
        ls();
        return;
    }
    if (text.substr(0,2) == "cd") {
        if (text.length() < 3) {
            addLine({"Invalid file target", RED, DS_SYS});
            return;
        }
        int i = 2;
        while (text[i++] == ' ') {
            if (i >= text.length()) {
                addLine({"Invalid file target", RED, DS_SYS});
                return;
            }
        }
        int startIndex = i-1;
        while (text[i] != ' ' && i++ < text.length()) {}
        cd(lines[start].text.substr(startIndex, i-startIndex));
    }
}

void Terminal::ls() {
    std::string response = "";
    for (int i=0; i < node.nChildren; i++) {
        response += node.children[i]->name + "  ";
    }
    addLine({response, BLUE, DS_SYS});
}

// TODO get this working with a full path
// keep reading until a /, then process the current step. stop on a space.
// If at any point you can't keep stepping forward, then go back to a saved
// node and send an error msg.
void Terminal::cd(std::string fileName) {
    if (fileName == "../") {
        node = *node.parent;
        return;
    }
    for (int i=0; i < node.nChildren; i++) {
        if (node.children[i]->name == fileName) {
            node = *node.children[i];
            return;
        }
    }
    addLine({"\"" + fileName + "\" doesn't exist", RED, DS_SYS});
}