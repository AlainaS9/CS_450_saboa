#include "student/VKMesh.hpp"

namespace student {

    void recordDrawVulkanMesh(vk::CommandBuffer &commandBuffer, VulkanMesh &mesh) {
        vk::Buffer vertexBuffers[] = {mesh.vertices.buffer};
        vk::DeviceSize offsets[] = {0};
        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(mesh.indices.buffer, 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(mesh.indexCnt, 1, 0, 0, 0);
    }

    void cleanupVulkanMesh(VulkanInitData &vkInitData, VulkanMesh &mesh) {
        cleanupVulkanBuffer(vkInitData, mesh.vertices);
        cleanupVulkanBuffer(vkInitData, mesh.indices);
    }

}