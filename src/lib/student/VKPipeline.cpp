#include "student/VKPipeline.hpp"

namespace student {

    vk::ShaderModule createVulkanShaderModule(
        VulkanInitData &vkInitData,
        const vector<char> &code
    ) {
        return vkInitData.device.createShaderModule(
            vk::ShaderModuleCreateInfo(
                vk::ShaderModuleCreateFlags(),
                code.size(),
                reinterpret_cast<const uint32_t*>(code.data())
            ));

    }

    void cleanupVulkanShaderModule(
        VulkanInitData &vkInitData,
        vk::ShaderModule &shaderModule
    ) {
        vkInitData.device.destroyShaderModule(shaderModule);

    }

    VulkanPipelineData createBasicVulkanPipeline(
        VulkanInitData &vkInitData,
        VulkanPipelineCreationInfo &creationInfo
    ) {
        VulkanPipelineData data {};

        auto vertShaderCode = readBinaryFile(creationInfo.vertSPVFilename);
        auto fragShaderCode = readBinaryFile(creationInfo.fragSPVFilename);

        vk::ShaderModule vertMod = createVulkanShaderModule(vkInitData, vertShaderCode);
        vk::ShaderModule fragMod = createVulkanShaderModule(vkInitData, fragShaderCode);

        vk::PipelineShaderStageCreateInfo vertInfo(
            {}, vk::ShaderStageFlagBits::eVertex, vertMod, "main"
        );

        vk::PipelineShaderStageCreateInfo fragInfo(
            {}, vk::ShaderStageFlagBits::eFragment, fragMod, "main"
        );

        vk::PipelineShaderStageCreateInfo shaderStages [] = {
            vertInfo, fragInfo
        };

        // BIG TODO

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
            {}, creationInfo.bindDesc, creationInfo.attribDesc
        );

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
            {}, vk::PrimitiveTopology::eTriangleList, false
        );


        cleanupVulkanShaderModule(vkInitData, vertMod);
        cleanupVulkanShaderModule(vkInitData, fragMod);

        return data;

    }

    void cleanupVulkanPipeline(
        VulkanInitData &vkINitData, 
        VulkanPipelineData &pipelineData
    ) {
        // TODO


    }


}