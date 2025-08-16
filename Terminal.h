//
// Created by Dylan Beaumont on 4/8/2025.
//

#ifndef TERMINAL_H
#define TERMINAL_H
#define MAX_LINES 200
#define MAX_COMMAND_LOOKBACK 5
#define LINE_SPACING 5.0f
#define DS_USER 0
#define DS_SYS 1
#include "Shader.h"
#include <glfw3.h>
#include <map>
#include "FileUtils.h"
#include "GameLayer.h"
#include "KeyState.h"

/* The terminal class will contain all of the text held by the buffer. This will be cycled
 * in a 2000 character ring buffer */

static const glm::vec3 GREEN = {0, 0.9, 0};
static const glm::vec3 BLUE = {0.4, 0.4, 0.9};
static const glm::vec3 RED = {0.9, 0.4, 0.4};
static const glm::vec3 WHITE = {0.8, 0.8, 0.8};


struct Character {
 unsigned int TextureID; // ID handle of the glyph texture
 glm::ivec2   Size;      // Size of glyph
 glm::ivec2   Bearing;// Offset from baseline to left/top of glyph
 unsigned char* Buffer;
 unsigned int Advance;   // Horizontal offset to advance to next glyph
};




struct Line {
    std::string text;
    glm::vec3 colour = BLUE;
    int side = DS_SYS;
};


class Terminal {
public:
  static Shader *shader;
  GLuint atlasTex;
  int* width;
  int* height;
  FileNode *node;
  FileNode *root;
  int cursor = 0;
  float lineHeight = 20;
  double viewHeight = 0;
  std::map<GLchar, Character> characters;
  Line lines[MAX_LINES]{};
  Line commands[MAX_COMMAND_LOOKBACK]{};
  int viewingCommand = 0;
  int atCommand = 0;
  std::string input;
  unsigned int end = -1;
  unsigned int start = -1;
  unsigned int window = -1;
  unsigned int VAO, VBO;
  float lHeight;
  bool active = false;
  std::function<void(GameLayer)> pushToStack;

  Terminal(FileNode root, FileNode node, std::function<void(GameLayer)> pushToStack, int *width, int *height);
  void processInput(GLFWwindow *window, KeyState *keyState, double deltaTime);
  int update(GLFWwindow* window, KeyState *keyState, double deltaTime);
  void renderBuffer(Shader shader, glm::vec2 pos, float width, float height);
  void renderText(Shader &shader, std::string text, float x, float y, float width, float lineHeight, float scale, glm::vec3 color);
  int getLineWraps(std::string_view text, float x, float width, float scale);
  void addLine(const Line& line);
  void stepBack();
  void autoComplete();
  void readCommand();
  void ls(const std::string_view path);
  void cd(const std::string_view path);
  void cat(std::string_view path);
};

#endif //TERMINAL_H
