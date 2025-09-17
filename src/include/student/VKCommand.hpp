#pragma once

#include <iostream>
#include <string>
#include "VKCore.hpp"
#include "student/VKSetup.hpp"

using namespace std;

namespace student {

    vk::CommandPool createVulkanCommandPool(VulkanInitData &vkInitData, unsigned int queueIndex);

    void cleanupVulkanCommandPool(VulkanInitData &vkInitData, vk::CommandPool &commandPool);

    vk::CommandBuffer createVulkanCommandBuffer(
                    VulkanInitData &vkInitData,
                    vk::CommandPool &commandPool
    );

    vk::Fence createVulkanFence(VulkanInitData &vkInitData);
    void cleanupVulkanFence(VulkanInitData &vkInitData, vk::Fence &f);

    vk::Semaphore createVulkanSemaphore(VulkanInitData &vkInitData);
    void cleanupVulkanSemaphore(VulkanInitData &vkInitData, vk::Semaphore &s);

    struct VulkanFIFCommandData {
        vk::CommandBuffer commandBuffer {};
        vk::Semaphore imageAvailSemaphore {};
        vk::Fence inFlightFence {};
    };

    struct VulkanCommandData {
        vk::CommandPool commandPool {};
        vector<vk::Semaphore> perSwapRenderDone {};
        vector<VulkanFIFCommandData> perFIF {};
        int numberFramesInFlight = 0;
    };

    VulkanCommandData createVulkanCommandData(VulkanInitData &vkInitData,  
                                            int numberFramesInFlight);
    void cleanupVulkanCommandData(VulkanInitData &vkInitData, VulkanCommandData &commandData); 

}