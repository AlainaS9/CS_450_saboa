#include "student/VKSetup.hpp"

namespace student {
    GLFWwindow* createGLFWWindow(string title, int width, int height, bool isResizable) {
    glfwInIt();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable);
    GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    return window;
}

    void cleanupGLFWwindow(GLFWwindow *window) {
        glfwDestroyWindow(window);
        glfwTerminate;
    }

}
