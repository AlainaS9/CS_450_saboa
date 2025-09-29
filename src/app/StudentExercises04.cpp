#include <iostream>
#include <string>
#include "student/VKSetup.hpp"

using namespace std;
using namespace student;

const char* getDeviceTypeString(vk::PhysicalDeviceType t) {
    switch(t) {
        case vk::PhysicalDeviceType::eDiscreteGpu: return "Discrete GPU";
        case vk::PhysicalDeviceType::eIntegratedGpu: return "Integrated GPU";
        case vk::PhysicalDeviceType::eVirtualGpu: return "Virtual GPU";
        case vk::PhysicalDeviceType::eCpu: return "CPU";
        default: return "Other";
    }
}

void printPhysicalDeviceProperties(vk::PhysicalDevice &pd) {
    vk::PhysicalDeviceProperties props = pd.getProperties();
    uint32_t api = props.apiVersion;

    cout << "Name: " << props.deviceName.data() << endl;
    cout << "Type: " << getDeviceTypeString(props.deviceType) << endl;
    cout << "API Version: " << VK_VERSION_MAJOR(api) << "."
                                << VK_VERSION_MINOR(api) << "."
                                << VK_VERSION_PATCH(api) << endl;
}

void listAvailablePhysicalDevices(VulkanInitData &vkInitData) {
    vector<vk::PhysicalDevice> allDev = vkInitData.instance.enumeratePhysicalDevices();
    cout << "Found " << allDev.size() << " physical devices:" << endl;
    for(int i = 0; i < allDev.size(); i++) {
        cout << "**DEVICE " << i << "***********" << endl;
        printPhysicalDeviceProperties(allDev.at(i));
    }
}

int main(int argc, char **argv) {
    cout << "BEGIN EXERCISES!!!!" << endl;

    string appName = "Exercises03";
    string windowTitle = appName;
    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow *window = createGLFWWindow(windowTitle, windowWidth, windowHeight);

    VulkanInitData vkInitData {};
    vkInitData.appName = appName;
    vkInitData.window = window;
    if(!createVulkanSetup(vkInitData)) {
        cleanupGLFWWindow(window);
        exit(1);
    }

    listAvailablePhysicalDevices(vkInitData);

    cout << "** Chosen Physical Device: ********" << endl;
    printPhysicalDeviceProperties(vkInitData.physicalDevice);

    uint32_t apiVersion = vk::enumerateInstanceVersion();
    cout << "Loader supports Vulkan " 
            << VK_VERSION_MAJOR(apiVersion) << "."
            << VK_VERSION_MINOR(apiVersion) << "."
            << VK_VERSION_PATCH(apiVersion) << endl;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    cleanupVulkanSetup(vkInitData);
    cleanupGLFWWindow(window);

    return 0;
}