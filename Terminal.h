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
#include "utils/FileUtils.h"
#include "GameLayer.h"
#include "Imp.h"
#include "KeyState.h"
#include "utils/TextUtil.h"

/* The terminal class will contain all of the text held by the buffer. This will be cycled
 * in a 2000 character ring buffer */


struct Line {
    std::string text;
    glm::vec3 colour = BLUE;
    int side = DS_SYS;
};


class Terminal {
public:
  Imp *imp;
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
  bool active = false;
  std::function<void(GameLayer)> pushToStack;

  Terminal(FileNode root, FileNode node, std::function<void(GameLayer)> pushToStack, int *width, int *height);
  void processInput(GLFWwindow *window, KeyState *keyState, double deltaTime);
  int update(GLFWwindow* window, KeyState *keyState, double deltaTime);
  void renderBuffer(Shader shader, glm::vec2 pos, float width, float height);
  void renderText(Shader &shader, std::string text, float x, float y, std::vector<int> lineWraps, float width, float lineHeight, float scale, glm::vec3 color);
  std::vector<int> getLineWraps(std::string_view text, float x, float width, float scale);
  void addLine(const Line& line);
  void stepBack();
  void autoComplete();
  void readCommand();
  void ls(const std::string_view path);
  void cd(const std::string_view path);
  void cat(std::string_view path);
};

#endif //TERMINAL_H
