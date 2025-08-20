#include <iostream>
#include <string>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>

#include "CylinderLock.h"
#include "GameLayer.h"
#include "Imp.h"
#include "KeyState.h"

#include FT_FREETYPE_H
#include "Shader.h"
#include "Terminal.h"
#include <unistd.h> // For Unix-like systems

// Stuff that needs to be done.
// TODO nicer wrapping
// TODO animations for cylinder lock.
// TODO split command args

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

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shaders
    Shader shader("../shaders/text.vert", "../shaders/text.frag");
    Terminal::shader = &shader;
    Shader lockShader("../shaders/lock.vert", "../shaders/lock.frag");
    CylinderLock::shader = &lockShader;
    Shader impShader("../shaders/imp.vert", "../shaders/imp.frag");
    Imp imp = Imp(&SCR_WIDTH, &SCR_HEIGHT);
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
        // TODO look into doing this with unique pointers instead.
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}


/*
 * Control resizing the framebuffer
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << "framebuffer callback\n";
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
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