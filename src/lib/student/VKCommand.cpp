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

    uint32_t prepareFrameInFlight(VulkanInitData &vkInitData, VulkanCommandData &commandData,
                                    uint32_t indexFIF) {

        vkInitData.device.waitForFences(
                        commandData.perFIF[indexFIF].inFlightFence, true, UINT64_MAX
                    );
        auto frameResult = vkInitData.device.acquireNextImageKHR(
                                vkInitData.swapchain.chain, UINT64_MAX,
                                commandData.perFIF[indexFIF].imageAvailSemaphore, nullptr
        );

        uint32_t indexSwap = frameResult.value;

        vkInitData.device.resetFences(commandData.perFIF[indexFIF].inFlightFence);

        commandData.perFIF[indexFIF].commandBuffer.reset();

        return indexSwap;

        }

        void submitToGraphicsQueue(VulkanInitData &vkInitData,
                                VulkanCommandData &commandData,
                                uint32_t indexFIF, uint32_t indexSwap) {
            vk::Semaphore waitSem [] = {
                commandData.perFIF[indexFIF].imageAvailSemaphore
            };

            vk::Semaphore signalSem [] = {
                commandData.perSwapRenderDone[indexSwap]
            };

            vk::PipelineStageFlags waitStages [] = {
                vk::PipelineStageFlagBits::eColorAttachmentOutput
            };

            vk::SubmitInfo submitInfo(waitSem, waitStages,
                                        commandData.perFIF[indexFIF].commandBuffer,
                                        signalSem);
            vkInitData.graphicsQueue.queue.submit(submitInfo, commandData.perFIF[indexFIF].inFlightFence);

        }

        bool presentSwapImage(VulkanInitData &vkInitData,
                                VulkanCommandData &commandData,
                                uint32_t indexFIF, uint32_t indexSwap) {

            vk::PresentInfoKHR presentInfo {};
            presentInfo.setWaitSemaphores(commandData.perSwapRenderDone[indexSwap]);
            presentInfo.setSwapchains(vkInitData.swapchain.chain);
            presentInfo.setImageIndices(indexSwap);

            bool successPresent = true;
            try {
                auto presResult = vkInitData.presentQueue.queue.presentKHR(presentInfo);
            }
            catch(vk::OutOfDateKHRError &e) {
                successPresent = false;
            }

            return successPresent;
        }


}
    
