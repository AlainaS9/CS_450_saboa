#pragma once

#include <iostream>
#include <string>
#include "VKCore.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "vk_mem_alloc.h"

using namespace std;

namespace student {

struct VulkanInitData {
    string appName = "";
    int minVersionMajor = 1;
    int minVersionMinor = 3;

    vkb::Instance bootInstance {};
    vk::Instance instance {};  //don't clean
    GLFWwindow *window = nullptr; //don't clean

    vk::SurfaceKHR surface {};

    vk::PhysicalDevice physicalDevice {}; //don't clean

    vkb::Device bootDevice {}; //don't clean
    vk::Device device {};
};

    bool createVulkanSetup(VulkanInitData &vkInitData);
    void cleanupVulkanSetup(VulkanInitData &vkInitData);

    GLFWwindow* createGLFWWindow(string title, int width, int height, bool isResizable = true);
    void cleanupGLFWWindow(GLFWwindow *window);

}