#pragma once

#include <iostream>
#include <string>
#include "student/VKSetup.hpp"
#include "student/VKBuffer.hpp"

using namespace std;

namespace student {

    template <typename T>
    struct HostMesh {
        vector<T> vertices;
        vector<unsigned int> indices {};
    };

    struct VulkanMesh {
        VulkanBuffer vertices {};
        VulkanBuffer indices {};
        unsigned int indexCnt = 0;
    };

    
}