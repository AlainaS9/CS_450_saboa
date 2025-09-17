#include "student/VKImage.hpp"

namespace student {

    VulkanImageTransition createVulkanImageTransition(
                        vk::Image &image, VK_IMAGE_TRANSITION_TYPE type
    ) {
        VulkanImageTransition transitionData {};

        vk::ImageLayout oldLayout {};
        vk::ImageLayout newLayout {};
        vk::AccessFlags srcMask {};
        vk::AccessFlags dstMask {};
        vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor;

        switch(type) {
            case UNDEF_TO_COLOR: {
                oldLayout = vk::ImageLayout::eUndefined;
                newLayout = vk::ImageLayout::eColorAttachmentOptimal;
                dstMask = vk::AccessFlagBits::eColorAttachmentWrite;
                transitionData.srcFlags = vk::PipelineStageFlagBits::eTopOfPipe;
                transitionData.dstFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                break;
            }
            case COLOR_TO_PRESENT: {
                oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
                newLayout = vk::ImageLayout::ePresentSrcKHR;
                srcMask = vk::AccessFlagBits::eColorAttachmentWrite;
                transitionData.srcFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                transitionData.dstFlags = vk::PipelineStageFlagBits::eBottomOfPipe;
                break;
            }
            default: {
                throw invalid_argument("Unsupported image transition!");
                break;
            }
        }

        vk::ImageMemoryBarrier barrier {};
        barrier.setOldLayout(oldLayout);
        barrier.setNewLayout(newLayout);
        barrier.setSrcAccessMask(srcMask);
        barrier.setDstAccessMask(dstMask);
        barrier.setImage(image);
        barrier.setSubresourceRange(vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1));

        transitionData.barrier = barrier;

        return transitionData;

    }

    void performVulkanImageTransition(vk::CommandBuffer &commandBuffer,
                                        VulkanImageTransition &transitionData) {
        commandBuffer.pipelineBarrier(
            transitionData.srcFlags, transitionData.dstFlags,
            {}, nullptr, nullptr,
            transitionData.barrier
        );
    
    }

}