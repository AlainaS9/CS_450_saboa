#pragma once

#include <iostream>
#include <string>
#include "VKCore.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>

using namespace std;

namespace student {
    GLFWwindow* createGLFWWindow(string title, int width, int height, bool isResizable = true);
    void cleanupGLFWwindow(GLFWwindow *window);

}