#pragma once

#include <iostream>
#include <string>
#include "VKCore.hpp"
#include "student/VKSetup.hpp"

using namespace std;

namespace student {

    vk::CommandPool createVulkanCommandPool(VulkanInitData &vkInitData, unsigned int queueIndex);

    void cleanupVulkanCommandPool(VulkanInitData &vkInitData, vk::CommandPool &commandPool);
    

}