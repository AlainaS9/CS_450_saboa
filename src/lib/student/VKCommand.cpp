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
    

}