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

        vk::Viewport viewport(
            0,
            0,
            (float)vkInitData.swapchain.extent.width,
            (float)vkInitData.swapchain.extent.height,
            0.0f,
            1.0f
        );

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, {},creationInfo.pushConstantRanges);

        vk::Rect2D scissor( {0,0}, vkInitData.swapchain.extent );

        vk::PipelineViewportStateCreateInfo viewportState({}, viewport, scissor);

        vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

        vk::PipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eNone;
        rasterizer.frontFace = vk::FrontFace::eCounterClockwise;


        vk::PipelineColorBlendAttachmentState colorBlendAttachment {}; 
        colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR
            | vk::ColorComponentFlagBits::eG
            | vk::ColorComponentFlagBits::eB
            | vk::ColorComponentFlagBits::eA;
            vk::PipelineColorBlendStateCreateInfo colorBlending({}, false, vk::LogicOp::eCopy, colorBlendAttachment);


        vk::PipelineDepthStencilStateCreateInfo depthStencil(
            {},
            true,
            true,
            vk::CompareOp::eLess,
            false,
            false,
            {}, {}
        );

        vk::PipelineLayoutCreateInfo layoutInfo(
            {}, 
            creationInfo.allDescSetLayouts, 
            creationInfo.pushConstantRanges);


        data.layout = vkInitData.device.createPipelineLayout(pipelineLayoutInfo);

        vk::PipelineMultisampleStateCreateInfo multisample({}, vk::SampleCountFlagBits::e1);

        data.cache = vkInitData.device.createPipelineCache(vk::PipelineCacheCreateInfo());

        vk::GraphicsPipelineCreateInfo pinfo {};
        pinfo.setFlags(vk::PipelineCreateFlags());
        pinfo.setStages(shaderStages);
        pinfo.setPVertexInputState(&vertexInputInfo);
        pinfo.setPInputAssemblyState(&inputAssembly);
        pinfo.setPViewportState(&viewportState);
        pinfo.setPRasterizationState(&rasterizer);
        pinfo.setPMultisampleState(&multisample);
        pinfo.setPDepthStencilState(&depthStencil);
        pinfo.setPColorBlendState(&colorBlending);
        pinfo.setPDynamicState(&dynamicState);
        pinfo.setLayout(data.layout);
        pinfo.setPNext(&(creationInfo.renderInfo));
        pinfo.setRenderPass(nullptr);
        auto ret = vkInitData.device.createGraphicsPipeline(data.cache, pinfo);

        if (ret.result != vk::Result::eSuccess) {
            throw runtime_error("Failed to create graphics pipeline!");
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
      /*        for(int i = 0; i < pipelineData.allDescSetLayouts.size(); i++) {
            vkInitData.device.destroyDescriptorSetLayout(
                pipelineData.allDescSetLayouts.at(i)
            );
            pipelineData.allDescSetLayouts.clear();
        }
            */

        vkInitData.device.destroyPipelineCache(pipelineData.cache);
        vkInitData.device.destroyPipelineLayout(pipelineData.layout);
        vkInitData.device.destroyPipeline(pipelineData.graphicsPipeline);

    }


}