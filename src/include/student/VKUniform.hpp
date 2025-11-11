#pragma once
#include <vector>
#include <cstddef>
#include "student/VKBuffer.hpp"

namespace student {
    struct UBOData {
        vector<VulkanBuffer> bufferData;
    };

    UBOData createVulkanUniformBufferData(
        VulkanInitData &vkInitData,
        size_t bufferSize,
        int maxFramesInFlight
    );

    void cleanupVulkanUniformBufferData(
        VulkanInitData &vkInitData,
        UBOData &uboData
    );


}