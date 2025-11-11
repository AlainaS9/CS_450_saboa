#include "student/VKUniform.hpp"

namespace student {

    UBOData createVulkanUniformBufferData(
        VulkanInitData &vkInitData,
        size_t bufferSize,
        int maxFramesInFlight
    ) {
        UBOData uboData {};
        uboData.bufferData.resize(maxFramesInFlight);

        for(int i = 0; i < maxFramesInFlight; i++) {
            uboData.bufferData[i] = createVulkanBuffer(
                vkInitData, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                createVMAHostVisibleInfo()
            );
        }

        return uboData;

    }

    void cleanupVulkanUniformBufferData(
        VulkanInitData &vkInitData,
        UBOData &uboData
    ) {

        for(int i = 0; i < uboData.bufferData.size(); i++) {
            cleanupVulkanBuffer(vkInitData, uboData.bufferData[i]);
        }
        uboData.bufferData.clear();

    }

}