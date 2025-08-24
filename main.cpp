#include <iostream>
#include <string>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>

#include "locks/CylinderLock.h"
#include "utils/GameLayer.h"
#include "imp/Imp.h"
#include "../utils/KeyState.h"

#include FT_FREETYPE_H
#include "utils/Shader.h"
#include "terminal/Terminal.h"
#include <unistd.h> // For Unix-like systems

#include "ui/MenuScreen.h"
#include "ui/TitleScreen.h"
#include "ui/UI.h"
#include "utils/SoundManager.h"

// Stuff that needs to be done.
// TODO animations for cylinder lock.
// TODO more flexible sound set up
// TODO save state
// TODO menu in game when press escape

GLFWwindow* setup();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;


int main()
{
    // GLFWwindow *window = setup();

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Daemon Shell", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    // glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // UI Textures
    TextureLoader textureLoader;
    unsigned int tex = textureLoader.loadTexture("../resources/ui/titleScreen.png");
    TitleScreen::tex = tex;
    MenuScreen::tex = tex;

    // SoundManager
    SoundManager *soundManager = new SoundManager();
    // Create shaders
    Shader shader("../shaders/text.vert", "../shaders/text.frag");
    Terminal::shader = &shader;
    Shader lockShader("../shaders/lock.vert", "../shaders/lock.frag");
    CylinderLock::shader = &lockShader;
    Shader impShader("../shaders/imp.vert", "../shaders/imp.frag");
    UI::shader = &impShader;
    Imp imp = Imp(&SCR_WIDTH, &SCR_HEIGHT);
    imp.soundManager = soundManager;
    imp.shader = &impShader;
    imp.textShader = &shader;
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));

    // Create keyState for handling inputs
    KeyState keyState = {};
    double deltaTime = 0.0f;
    double windowTime = 0.01667f;

    std::stack<GameLayer> gameStack = {};

    // Create terminal
    FileNode *root = readSystem("../root.json");
    Terminal terminal = Terminal(*root, *root, [&gameStack](GameLayer gl) {gameStack.push(gl);}, &SCR_WIDTH, &SCR_HEIGHT);
    terminal.imp = &imp;
    gameStack.push(
        { [&terminal](GLFWwindow *_window, KeyState *_keyState, double _deltaTime){return terminal.update(_window, _keyState, _deltaTime);},
        [&terminal]() {return;} }
    );

    // Create title screen
    TitleScreen titleScreen = TitleScreen(&SCR_WIDTH, &SCR_HEIGHT);
    gameStack.push(
        {[&titleScreen](GLFWwindow *_window, KeyState *_keyState, double _deltaTime){return titleScreen.update(_window, _deltaTime);},
        [&titleScreen]() {return;}}
        );

    usleep(500);
    // START MAIN LOOP //
    while (!glfwWindowShouldClose(window))
    {
        double curTime = glfwGetTime();
        deltaTime = curTime-windowTime;
        windowTime = glfwGetTime();
        // std::cout << 1.0 / deltaTime << " FPS\n";
        processInput(window);

        glClearColor(0.0f, 0.05f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
        // todo Look into why I need this + make it cleaner
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lockShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(impShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        if (gameStack.top().update(window, &keyState, deltaTime)) {
            gameStack.top().cleanup();
            gameStack.pop();
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // END MAIN LOOP //

    glfwTerminate();
    return 0;
}


/*
 * This creates the glfw window and establishes some basic settings
 */
inline GLFWwindow* setup()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Daemon Shell", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return window;
}


/*
 * This handles escaping the window/game
 */
void processInput(GLFWwindow *window)
{
    // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    //     glfwSetWindowShouldClose(window, true);
    // }
}


/*
 * Control resizing the framebuffer
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


/*
 * Control resizing the window
 */
void window_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}