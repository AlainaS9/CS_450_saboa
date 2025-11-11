#include "student/VKImage.hpp"

namespace student {

        VulkanImage createVulkanImage(
        VulkanInitData &vkInitData,
        vk::Extent3D extent,
        vk::Format format,
        vk::ImageUsageFlags usage,
        vk::ImageAspectFlags aspectFlags,
        uint32_t mipLevels,
        vk::SampleCountFlagBits samples
    ) {
        VulkanImage imageData {};
        imageData.extent = extent;
        imageData.format = format;
        imageData.mipLevels = mipLevels;

        vk::ImageCreateInfo imgInfo {};
        imgInfo.extent = extent;
        imgInfo.mipLevels = mipLevels;
        imgInfo.samples = samples;
        imgInfo.format = format;
        imgInfo.usage = usage;
        imgInfo.imageType = vk::ImageType::e2D;
        imgInfo.initialLayout = vk::ImageLayout::eUndefined;
        imgInfo.arrayLayers = 1;
        imgInfo.tiling = vk::ImageTiling::eOptimal;
        imgInfo.sharingMode = vk::SharingMode::eExclusive;

        VmaAllocationCreateInfo vmaInfo {};
        vmaInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VkImageCreateInfo vkCreateInfo = static_cast<VkImageCreateInfo>(imgInfo);
        VkImage vkImage {};
        VmaAllocation alloc {};
        vmaCreateImage(vkInitData.allocator, &vkCreateInfo, &vmaInfo, &vkImage, &alloc, nullptr);

        imageData.image = vk::Image { vkImage };
        imageData.allocation = alloc;

        vk::ImageViewCreateInfo viewInfo {};
        viewInfo.image = imageData.image;
        viewInfo.format = format;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.subresourceRange = { aspectFlags, 0, 1, 0, 1 };

        imageData.view = vkInitData.device.createImageView(viewInfo);

        return imageData;
    }

    void cleanupVulkanImage(
        VulkanInitData &vkInitData,
        VulkanImage &imageData
    ) {
        vkInitData.device.destroyImageView(imageData.view);
        vmaDestroyImage(vkInitData.allocator, 
                        imageData.image,
                        imageData.allocation);
    }

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
            case UNDEF_TO_DEPTH: {
                oldLayout = vk::ImageLayout::eUndefined;
                newLayout = vk::ImageLayout::eDepthAttachmentOptimal;
                dstMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | 
                            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                transitionData.srcFlags = vk::PipelineStageFlagBits::eTopOfPipe;
                transitionData.dstFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests |
                                            vk::PipelineStageFlagBits::eLateFragmentTests;
                aspectFlags = vk::ImageAspectFlagBits::eDepth;
                break;
            }
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

    
    void recreateAllVulkanDepthImages(VulkanInitData &vkInitData,
                                        vk::CommandBuffer &commandBuffer,
                                        vector<VulkanImage> &allDepthImages) {

        if(allDepthImages.size() > 0) {
            cleanupAllVulkanDepthImages(vkInitData, allDepthImages);
        }

        for(int i = 0; i < vkInitData.swapchain.images.size(); i++) {
            VulkanImage depthImage = createVulkanImage(
                vkInitData,
                { vkInitData.swapchain.extent.width,
                  vkInitData.swapchain.extent.height,
                  1  },
                vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment,
                vk::ImageAspectFlagBits::eDepth, 1, vk::SampleCountFlagBits::e1
            );
            allDepthImages.push_back(depthImage);
            VulkanImageTransition depthTransition = createVulkanImageTransition(
                depthImage.image, VK_IMAGE_TRANSITION_TYPE::UNDEF_TO_DEPTH
            );
            performVulkanImageTransition(commandBuffer, depthTransition);
        }
    }                                  
        
                                        

    void cleanupAllVulkanDepthImages(VulkanInitData &vkInitData,
                                        vector<VulkanImage> &allDepthImages) {
        for(int i = 0; i < allDepthImages.size(); i++) {
            cleanupVulkanImage(vkInitData, allDepthImages.at(i));
        }
                                        }
        

}