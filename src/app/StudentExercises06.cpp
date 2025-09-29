#include <iostream>
#include <string>
#include "student/VKSetup.hpp"
#include "student/VKCommand.hpp"
#include "student/VKImage.hpp"
#include "student/VKPipeline.hpp"

using namespace std;
using namespace student;

struct ForgeVertex {
glm::vec3 pos;
};

void recordCommands (VulkanInitData &vkInitData, uint32_t indexFIF, uint32_t indexSwap,
                        vk::CommandBuffer &commandBuffer, 
                        vk::QueryPool &queryPool) {
    commandBuffer.begin(vk::CommandBufferBeginInfo());

    commandBuffer.resetQueryPool(queryPool, 0, 2);
    commandBuffer.writeTimestamp2(vk::PipelineStageFlagBits2::eTopOfPipe, 
                                    queryPool, 0);

    VulkanImageTransition colorBarrier = createVulkanImageTransition(
                                      vkInitData.swapchain.images[indexSwap],
                                      VK_IMAGE_TRANSITION_TYPE::UNDEF_TO_COLOR
    );
    performVulkanImageTransition(commandBuffer, colorBarrier);

    // TODO NOW REAL RENDERING COMMANDS

    VulkanImageTransition presentBarrier = createVulkanImageTransition(
                                      vkInitData.swapchain.images[indexSwap],
                                      VK_IMAGE_TRANSITION_TYPE::COLOR_TO_PRESENT
    );
    performVulkanImageTransition(commandBuffer, presentBarrier);
    
    commandBuffer.writeTimestamp2(vk::PipelineStageFlagBits2::eBottomOfPipe, queryPool, 1);

    commandBuffer.end();
}

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

    string appName = "StudentExercises06";
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

    int numberFramesInFlight = 2;
    VulkanCommandData commandData = createVulkanCommandData(vkInitData, numberFramesInFlight);

    uint64_t framesRendered = 0;

    vk::QueryPoolCreateInfo qpci {};
    qpci.queryType = vk::QueryType::eTimestamp;
    qpci.queryCount = 2;
    vector<vk::QueryPool> queryPools {};
    for(int i = 0; i < commandData.numberFramesInFlight; i++) {
        queryPools.push_back(vkInitData.device.createQueryPool(qpci));
    }

    VulkanPipelineCreationInfo pipeInfo {};
    pipeInfo.vertSPVFilename = "build/compiledshaders/" + appName + "/shader.vert.spv";
    pipeInfo.fragSPVFilename = "build/compiledshaders/" + appName + "/shader.frag.spv";
    pipeInfo.bindDesc = vk::VertexInputBindingDescription(
        0, sizeof(ForgeVertex), vk::VertexInputRate::eVertex
    );
    pipeInfo.attribDesc.push_back(vk::VertexInputAttributeDescription(
        0, //location
        0, //binding
        vk::Format::eR32G32B32Sfloat, //format
        offsetof(ForgeVertex, pos) //offset
    )
    );
    // TODO 
    VulkanPipelineData pipelineData = createBasicVulkanPipeline(vkInitData, pipeInfo);


    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t indexFIF = framesRendered % numberFramesInFlight;
        uint32_t indexSwap = prepareFrameInFlight(vkInitData, commandData, indexFIF);

        recordCommands(vkInitData, indexFIF, indexSwap, commandData.perFIF[indexFIF].commandBuffer,
                        queryPools[indexFIF]);

        submitToGraphicsQueue(vkInitData, commandData, indexFIF, indexSwap);

        if(!presentSwapImage(vkInitData, commandData, indexFIF, indexSwap)) {
            recreateVulkanSwapchain(vkInitData);
        }

        framesRendered++;

        uint64_t timeStamps[2] = {};
        vkInitData.device.getQueryPoolResults(queryPools[indexFIF], 0, 2,
                            sizeof(timeStamps), timeStamps, sizeof(uint64_t),
                            vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);

        auto props = vkInitData.physicalDevice.getProperties();
        double nsPerTick = props.limits.timestampPeriod;
        double deltaNs = (timeStamps[1] - timeStamps[0])*nsPerTick;

      //  cout << "TIME PER FIF " << indexFIF << ": " << deltaNs << endl;
        }

    vkInitData.device.waitIdle();

    cleanupVulkanPipeline(vkInitData, pipelineData);

    for(int i = 0; i < queryPools.size(); i++) {
        vkInitData.device.destroyQueryPool(queryPools[i]);
    }
    queryPools.clear();

    cleanupVulkanCommandData(vkInitData, commandData);
    cleanupVulkanSetup(vkInitData);
    cleanupGLFWWindow(window);

    return 0;
}