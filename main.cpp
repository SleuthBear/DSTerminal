#include <iostream>
#include <string>
#include <glad/glad.h>
// #include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>

#include "CylinderLock.h"
#include "KeyState.h"

#include FT_FREETYPE_H
#include "Shader.h"
#include "Terminal.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Terminal *terminal, KeyState *keyState, double deltaTime);
void charCallback(GLFWwindow *window, unsigned int codepoint);
void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Daemon Shell", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // compile and setup the shader
    // ----------------------------
    Shader shader("../shaders/text.vert", "../shaders/text.frag");
    Shader lockShader("../shaders/lock.vert", "../shaders/lock.frag");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    // shader.use();
    lockShader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    FileNode *root = readSystem("../root.json");
    Terminal terminal = Terminal(*root, *root);
    glfwSetWindowUserPointer(window, &terminal);
    glfwSetCharCallback(window, charCallback);
    glfwSetScrollCallback(window, scrollCallback);
    // render loop
    // -----------
    KeyState keyState = {}; // Start with everything off.
    bool enterPressed = false;
    double deltaTime = 0.0f;
    double windowTime = -0.1667f;
    CylinderLock lock = CylinderLock(5, 1.0, SCR_WIDTH/2, SCR_HEIGHT/2, 100);
    while (!glfwWindowShouldClose(window))
    {
        double curTime = glfwGetTime();
        deltaTime = curTime-windowTime;
        windowTime = glfwGetTime();
        // input
        // -----
        processInput(window, &terminal, &keyState, deltaTime);
        lock.processInput(window, &keyState, deltaTime);
        // render
        // ------
        glm::vec3 color = {0, 1, 0};
        projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lockShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // glUniformMatrix3fv(glGetUniformLocation(lockShader.ID, "color"), 1, GL_FALSE, glm::value_ptr(color));

        glClearColor(0.0f, 0.05f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        lock.rotateLock(deltaTime);
        lock.render(lockShader);
        terminal.renderBuffer(shader, {5.0f, 5.0f}, (float)SCR_WIDTH-15.0f, (float)SCR_HEIGHT-15.0f);
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Terminal *terminal, KeyState *keyState, double deltaTime)
{
    keyState->addDeltaTime(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        if (keyState->backspace > keyState->interval) {
            unsigned int start = terminal->start;
            std::string *input = &terminal->lines[start].text;
            if (!input->empty()) {
                input->erase(input->length()-1);
            }
            if (terminal->cursor > 0) {
                terminal->cursor--;
            }
            keyState->backspace = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        if (keyState->enter > keyState->interval) {
            if (!terminal->lines[terminal->start].text.empty()) {
                terminal->readCommand();
                terminal->addLine({"", GREEN, DS_USER});
                terminal->viewHeight = 0;
                terminal->cursor = 0;
            }
            keyState->enter = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && terminal->cursor > 0) {
        if (keyState->left > keyState->interval) {
            terminal->cursor--;
            keyState->left = 0;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && terminal->cursor < terminal->lines[terminal->start].text.length()) {
        if (keyState->right > keyState->interval) {
            terminal->cursor++;
            keyState->right = 0;
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
    if(start > end && (start - end) < (double)SCR_HEIGHT/lineHeight) {
        return;
    }
    yPos += yOffset;
    yPos = std::max(0.0, yPos);
    int availableLines = MAX_LINES;
    if (start > end) {
        availableLines = start-end;
    }
    // Must always have a full screen of lines available
    yPos = std::min((double)availableLines*lineHeight - (double)SCR_HEIGHT, yPos);
    printf("%f\n", yPos);
    terminal->window = static_cast<unsigned int>(static_cast<double>(start) - yPos/lineHeight);
    terminal->viewHeight = yPos;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void window_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}