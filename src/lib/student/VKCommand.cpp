#include "student/VKCommand.hpp"

namespace student {

    vk::CommandPool createVulkanCommandPool(VulkanInitData &vkInitData, unsigned int queueIndex) {
        return vkInitData.device.createCommandPool(
            vk::CommandPoolCreateInfo(
                vk::CommandPoolCreateFlags(
                    vk::CommandPoolCreateFlagBits::eResetCommandBuffer
                ),
                queueIndex
            )
    );

    }

    void cleanupVulkanCommandPool(VulkanInitData &vkInitData, vk::CommandPool &commandPool) {

        vkInitData.device.destroyCommandPool(commandPool);

    }

    vk::CommandBuffer createVulkanCommandBuffer(
                    VulkanInitData &vkInitData,
                    vk::CommandPool &commandPool
        ) {
            return vkInitData.device.allocateCommandBuffers(
                    vk::CommandBufferAllocateInfo(
                        commandPool,
                        vk::CommandBufferLevel::ePrimary,
                        1
                    )
            ).front();
        }

    vk::Fence createVulkanFence(VulkanInitData &vkInitData) {
        return vkInitData.device.createFence(
            vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)
        );

    }

    void cleanupVulkanFence(VulkanInitData &vkInitData, vk::Fence &f) {
        vkInitData.device.destroyFence(f);

    }

    vk::Semaphore createVulkanSemaphore(VulkanInitData &vkInitData){
        return vkInitData.device.createSemaphore(vk::SemaphoreCreateInfo());
    }

    void cleanupVulkanSemaphore(VulkanInitData &vkInitData, vk::Semaphore &s) {
        vkInitData.device.destroySemaphore(s);
    }

    VulkanCommandData createVulkanCommandData(VulkanInitData &vkInitData,  
                                            int numberFramesInFlight) {
        VulkanCommandData data {};
        data.commandPool = createVulkanCommandPool(vkInitData, vkInitData.graphicsQueue.index);
        data.numberFramesInFlight = numberFramesInFlight;

        for(int i = 0; i < vkInitData.swapchain.images.size(); i++) {
            data.perSwapRenderDone.push_back(createVulkanSemaphore(vkInitData));
        }

        for(int i = 0; i < numberFramesInFlight; i++) {
            VulkanFIFCommandData cd {};
            cd.commandBuffer = createVulkanCommandBuffer(vkInitData, data.commandPool);
            cd.imageAvailSemaphore = createVulkanSemaphore(vkInitData);
            cd.inFlightFence = createVulkanFence(vkInitData);
            data.perFIF.push_back(cd);
        }
        return data;
    }

    void cleanupVulkanCommandData(VulkanInitData &vkInitData, VulkanCommandData &commandData) {
        for(int i = 0; i < commandData.perFIF.size(); i++) {
            cleanupVulkanSemaphore(vkInitData, commandData.perFIF.at(i).imageAvailSemaphore);
            cleanupVulkanFence(vkInitData, commandData.perFIF.at(i).inFlightFence);
        }

        commandData.perFIF.clear();

        for(int i = 0; i < commandData.perSwapRenderDone.size(); i++) {
            cleanupVulkanSemaphore(vkInitData, commandData.perSwapRenderDone.at(i));
        }
        commandData.perSwapRenderDone.clear();
        commandData.numberFramesInFlight = 0;
        cleanupVulkanCommandPool(vkInitData, commandData.commandPool);
    } 


}
    
