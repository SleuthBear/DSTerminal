//
// Created by Dylan Beaumont on 4/8/2025.
//

#ifndef TERMINAL_H
#define TERMINAL_H
#define MAX_LINES 200
#define LINE_SPACING 5.0f
#define DS_USER 0
#define DS_SYS 1
#include "Shader.h"
#include <glfw3.h>
#include <map>

/* The terminal class will contain all of the text held by the buffer. This will be cycled
 * in a 2000 character ring buffer */

static const glm::vec3 GREEN = {0, 0.9, 0};
static const glm::vec3 BLUE = {0.4, 0.4, 0.9};
static const glm::vec3 RED = {0.9, 0.4, 0.4};

struct Character {
 unsigned int TextureID; // ID handle of the glyph texture
 glm::ivec2   Size;      // Size of glyph
 glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
 unsigned int Advance;   // Horizontal offset to advance to next glyph
};

struct FileNode {
    std::string name;
    int type;
    FileNode *parent;
    FileNode **children;
    int nChildren;
};

struct Line {
    std::string text;
    glm::vec3 colour = BLUE;
    int side = DS_SYS;
};


class Terminal {
public:
  // col and row pos
  FileNode node;
  FileNode root;
  int cursor = 0;
  float lineHeight = 20;
  double viewHeight = 0;
  std::map<GLchar, Character> characters;
  Line lines[MAX_LINES]{};
  std::string input;
  unsigned int end = -1;
  unsigned int start = -1;
  unsigned int window = -1;
  unsigned int VAO, VBO;
  // height of l character
  float lHeight;
  Terminal(FileNode root, FileNode node);
  void renderBuffer(Shader shader, glm::vec2 pos, float width, float height);
  void renderText(Shader &shader, std::string text, float x, float y, float width, float lineHeight, float scale, glm::vec3 color);
  int getLineWraps(std::string text, float x, float width, float scale);
  void addLine(const Line& line);
  void readCommand();
  void ls();
  void cd(std::string);
};

#endif //TERMINAL_H
