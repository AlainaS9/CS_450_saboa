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

    template <typename T>
    VulkanMesh createVulkanMesh(
        VulkanInitData &vkInitData,
        HostMesh<T> &hostMesh,
        bool useStaging
    ) {
        VulkanMesh vmesh {};

        VmaAllocationCreateInfo vmaCreateInfo {};
        vk::BufferUsageFlags vertFlags = vk::BufferUsageFlagBits::eVertexBuffer;
        vk::BufferUsageFlags indexFlags = vk::BufferUsageFlagBits::eIndexBuffer;

        if(useStaging) {
            vmaCreateInfo = createVMADeviceLocalInfo();
            vertFlags |= vk::BufferUsageFlagBits::eTransferDst;
            indexFlags |= vk::BufferUsageFlagBits::eTransferDst;
        }
        else {
            vmaCreateInfo = createVMAHostVisibleInfo();
        }

        vk::DeviceSize vertSize = sizeof(hostMesh.vertices[0])*hostMesh.vertices.size();
        vk::DeviceSize indexSize = sizeof(hostMesh.indices[0])*hostMesh.indices.size();

        vmesh.vertices = createVulkanBuffer(vkInitData, vertSize, vertFlags, vmaCreateInfo);
        vmesh.indices = createVulkanBuffer(vkInitData, indexSize, indexFlags, vmaCreateInfo);

        return vmesh;
    }

    template<typename T>
    void copyToVulkanMesh(
                VulkanInitData &vkInitData, 
                VulkanMesh &mesh,
                HostMesh<T> &hostMesh,
                bool useStaging,
                VulkanStagingData &stagingData = nullptr) {
        if(useStaging) {
            copyToDeviceLocalVulkanBuffer(
                vkInitData, stagingData, 
                mesh.vertices, hostMesh.vertices.data());
            copyToDeviceLocalVulkanBuffer(
                vkInitData, stagingData, 
                mesh.indices, hostMesh.indices.data());
        }
        else {
            copyToHostVisibleVulkanBuffer(
                vkInitData, mesh.vertices, hostMesh.vertices.data());
            copyToHostVisibleVulkanBuffer(
                vkInitData, mesh.indices, hostMesh.indices.data());
        }
        mesh.indexCnt = hostMesh.indices.size();
    }

    void recordDrawVulkanMesh(vk::CommandBuffer &commandBuffer, VulkanMesh &mesh);

    void cleanupVulkanMesh(VulkanInitData &vkInitData, VulkanMesh &mesh);
    
}