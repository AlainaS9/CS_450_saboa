#include "student/VKBuffer.hpp"

namespace student {

    VmaAllocationCreateInfo createVMAHostVisibleInfo() {
        VmaAllocationCreateInfo info {};
        info.usage = VMA_MEMORY_USAGE_AUTO;
        info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT;
        return info;
    }

    VmaAllocationCreateInfo createVMADeviceLocalInfo() {
        VmaAllocationCreateInfo info {};
        info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        return info;
    }

    VulkanBuffer createVulkanBuffer(VulkanInitData &vkInitData,
                                    vk::DeviceSize size, 
                                    vk::BufferUsageFlags usage,
                                    VmaAllocationCreateInfo vmaInfo) {
        VulkanBuffer vbuffer {};
        vbuffer.size = size;
        vbuffer.usage = usage;

        VkBufferCreateInfo bci { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bci.size = size;
        bci.usage = static_cast<VkBufferUsageFlags>(usage);
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer buffer {};
        VmaAllocation allocation {};
        VmaAllocationInfo allocInfo {};
        vmaCreateBuffer(vkInitData.allocator, &bci, &vmaInfo,
                         &buffer, &allocation, &allocInfo);

        vbuffer.buffer = vk::Buffer { buffer };
        vbuffer.allocation = allocation;
        vbuffer.mapped = allocInfo.pMappedData;

        return vbuffer;
    }

    void cleanupVulkanBuffer(VulkanInitData &vkInitData, VulkanBuffer &buffer) {
        if(buffer.buffer) {
            vmaDestroyBuffer(
                vkInitData.allocator, 
                static_cast<VkBuffer>(buffer.buffer), 
                buffer.allocation);
            buffer = VulkanBuffer {};
        }
    }

    void copyToHostVisibleVulkanBuffer(
        VulkanInitData &vkInitData,
        VulkanBuffer &bufferData,
        void *hostData
    ) {
        memcpy(bufferData.mapped, hostData, bufferData.size);
        vmaFlushAllocation(vkInitData.allocator, bufferData.allocation,
                            0, VK_WHOLE_SIZE);

    }

    VulkanStagingData beginStagingVulkanBufferCopies(VulkanInitData &vkInitData,
                                                    vk::CommandPool &commandPool) {

        VulkanStagingData stagingData {};
        stagingData.commandBuffer = createVulkanCommandBuffer(vkInitData, commandPool);

        stagingData.commandBuffer.begin(
            vk::CommandBufferBeginInfo(
                vk::CommandBufferUsageFlagBits::eOneTimeSubmit
            )
        );

        return stagingData;
    }
    
    void copyToDeviceLocalVulkanBuffer(
        VulkanInitData &vkInitData,
        VulkanStagingData &stagingData,
        VulkanBuffer &bufferData,
        void* hostData
    ) {
        VulkanBuffer stageData = createVulkanBuffer(vkInitData, bufferData.size,
                                                    vk::BufferUsageFlagBits::eTransferSrc,
                                                    createVMAHostVisibleInfo());
        copyToHostVisibleVulkanBuffer(vkInitData, stageData, hostData);

        vk::BufferCopy copyRegion {};
        copyRegion.size = bufferData.size;
        stagingData.commandBuffer.copyBuffer(
            stageData.buffer, 
            bufferData.buffer, 
            copyRegion);

        stagingData.allTempBuffers.push_back(stageData);

    }

    void endStagingVulkanBufferCopies(
        VulkanInitData &vkInitData,
        vk::CommandPool commandPool,
        VulkanStagingData &stagingData
    ) {
        stagingData.commandBuffer.end();
        vk::SubmitInfo submitInfo = vk::SubmitInfo().setCommandBuffers(stagingData.commandBuffer);
        vkInitData.graphicsQueue.queue.submit(submitInfo);
        vkInitData.graphicsQueue.queue.waitIdle();

        vkInitData.device.freeCommandBuffers(commandPool, stagingData.commandBuffer);
        for(int i = 0; i < stagingData.allTempBuffers.size(); i++) {
            cleanupVulkanBuffer(vkInitData, stagingData.allTempBuffers.at(i));
        }

        stagingData.allTempBuffers.clear();

    }

}