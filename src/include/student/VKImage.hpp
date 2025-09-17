#pragma once

#include <iostream>
#include <string>
#include "VKCore.hpp"
#include "student/VKSetup.hpp"

using namespace std;

namespace student {

    struct VulkanImageTransition {
        vk::ImageMemoryBarrier barrier {};
        vk::PipelineStageFlags srcFlags {};
        vk::PipelineStageFlags dstFlags {};
    };

    enum VK_IMAGE_TRANSITION_TYPE {
        UNDEF_TO_COLOR,
        COLOR_TO_PRESENT
    };

    VulkanImageTransition createVulkanImageTransition(
                        vk::Image &image, VK_IMAGE_TRANSITION_TYPE type
    );
    void performVulkanImageTransition(vk::CommandBuffer &commandBuffer,
                                        VulkanImageTransition &transitionData);


}