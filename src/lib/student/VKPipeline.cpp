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

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
            {}, creationInfo.bindDesc, creationInfo.attribDesc
        );

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
            {}, vk::PrimitiveTopology::eTriangleList, false
        );

        vk::Viewport viewport(0,0,
                                (float)vkInitData.swapchain.extent.width,
                                (float)vkInitData.swapchain.extent.height,
                                0.0f, 1.0f);

        vk::Rect2D scissors({0,0}, vkInitData.swapchain.extent);

        vk::PipelineViewportStateCreateInfo viewportInfo({}, viewport, scissors);

        vector<vk::DynamicState> dynamicList = {
                    vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamicInfo({}, dynamicList);

        vk::PipelineRasterizationStateCreateInfo rasterInfo {};
        rasterInfo.lineWidth = 1.0f;
        rasterInfo.cullMode = vk::CullModeFlagBits::eNone; // eBack
        rasterInfo.frontFace = vk::FrontFace::eCounterClockwise;

        vk::PipelineColorBlendAttachmentState blendAttach {};
        blendAttach.colorWriteMask =   
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo blendInfo(
            {}, false, vk::LogicOp::eCopy, blendAttach
        );

        vk::PipelineDepthStencilStateCreateInfo depthInfo(
            {},
            true, true,
            vk::CompareOp::eLess,
            false, 
            false, {}, {}
        );

        vk::PipelineLayoutCreateInfo layoutInfo({}, {}, {});
        data.layout = vkInitData.device.createPipelineLayout(layoutInfo);

        vk::PipelineMultisampleStateCreateInfo multiInfo(
            {}, vk::SampleCountFlagBits::e1);

        data.cache = vkInitData.device.createPipelineCache({});

        vk::GraphicsPipelineCreateInfo pinfo {};
        pinfo.setFlags({});
        pinfo.setStages(shaderStages);
        pinfo.setPVertexInputState(&vertexInputInfo);
        pinfo.setPInputAssemblyState(&inputAssembly);
        pinfo.setPViewportState(&viewportInfo);
        pinfo.setPRasterizationState(&rasterInfo);
        pinfo.setPMultisampleState(&multiInfo);
        pinfo.setPDepthStencilState(&depthInfo);
        pinfo.setPColorBlendState(&blendInfo);
        pinfo.setPDynamicState(&dynamicInfo);
        pinfo.setLayout(data.layout);
        pinfo.setPNext(&(creationInfo.renderInfo));
        pinfo.setRenderPass(nullptr);

        auto ret = vkInitData.device.createGraphicsPipeline(data.cache, pinfo);

        if(ret.result != vk::Result::eSuccess) {
            throw runtime_error("Error creating graphics pipeline!");
        }

        data.graphicsPipeline = ret.value;

        cleanupVulkanShaderModule(vkInitData, vertMod);
        cleanupVulkanShaderModule(vkInitData, fragMod);

        return data;

    }

    void cleanupVulkanPipeline(
        VulkanInitData &vkInitData, 
        VulkanPipelineData &pipelineData
    ) {
        // TODO
        vkInitData.device.destroyPipelineCache(pipelineData.cache);
        vkInitData.device.destroyPipelineLayout(pipelineData.layout);
        vkInitData.device.destroyPipeline(pipelineData.graphicsPipeline);

    }


}