#include "student/VKSetup.hpp"

namespace student {

        bool getVulkanQueue(vkb::Device vkbDevice,
                        vkb::QueueType queueType,
                        VulkanQueue &queueData) {

            // Get the desired queue
            auto qRet = vkbDevice.get_queue(queueType);
            if(!qRet) {
                cerr << "ERROR: " << qRet.error().message() << endl;
                return false;
            }

            queueData.queue = vk::Queue { qRet.value() };
            queueData.index = vkbDevice.get_queue_index(queueType).value();

            return true;
        }

    bool createVulkanSwapchain(VulkanInitData &vkInitData) {
        vkb::SwapchainBuilder swapBuilder { vkInitData.bootDevice };
        VkSurfaceFormatKHR format;
        format.format = VK_FORMAT_B8G8R8A8_UNORM;
        format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        auto swapRet = swapBuilder.set_desired_format(format).build();
        if(!swapRet) {
            cerr << "ERROR: " << swapRet.error().message() << endl;
            return false;
        }

        vkb::Swapchain vkbSwapchain = swapRet.value();
        vkInitData.swapchain.chain = vk::SwapchainKHR { vkbSwapchain.swapchain };
        vkInitData.swapchain.extent = vk::Extent2D { vkbSwapchain.extent };
        vkInitData.swapchain.format = vk::Format { vkbSwapchain.image_format };

        vector<VkImage> vkImages = vkbSwapchain.get_images().value();
        vector<VkImageView> vkViews = vkbSwapchain.get_image_views().value();
        for(int i = 0; i < vkImages.size(); i++) {
            vkInitData.swapchain.images.push_back(vk::Image { vkImages.at(i) });
            vkInitData.swapchain.views.push_back(vk::ImageView { vkViews.at(i) });
        }

        return true;

    }
    void recreateVulkanSwapchain(VulkanInitData &vkInitData) {
        vkInitData.device.waitIdle();
        cleanupVulkanSwapchain(vkInitData);
        createVulkanSwapchain(vkInitData);


    }
    void cleanupVulkanSwapchain(VulkanInitData &vkInitData) {
        for(int i = 0; i < vkInitData.swapchain.views.size(); i++ ) {
            vkInitData.device.destroyImageView(vkInitData.swapchain.views.at(i));
        }
        vkInitData.swapchain.views.clear();
        vkInitData.swapchain.images.clear();
        vkInitData.swapchain.extent = vk::Extent2D {};
        vkInitData.swapchain.format = vk::Format {};
        vkInitData.device.destroySwapchainKHR(vkInitData.swapchain.chain);

    }

    bool createVulkanSetup(VulkanInitData &vkInitData) {
        vkb::InstanceBuilder builder;
        auto instRet = builder.set_app_name(vkInitData.appName.c_str())
                                            .set_engine_name("Forge Engine")
                                            .request_validation_layers()
                                            .use_default_debug_messenger()
                                            .require_api_version(
                                                vkInitData.minVersionMajor,
                                                vkInitData.minVersionMinor,
                                                0
                                            ).build();
        if(!instRet) {
            cerr << "ERROR: " << instRet.error().message() << endl;
            return false;
        }

        vkInitData.bootInstance = instRet.value();
        vkInitData.instance = vk::Instance { vkInitData.bootInstance.instance };

        //SURFACE
        VkSurfaceKHR surface = nullptr;
        VkResult surfRet = glfwCreateWindowSurface(vkInitData.bootInstance.instance,
                                                    vkInitData.window, nullptr,
                                                     &surface);

        if(surfRet != VK_SUCCESS) {
            cerr << "ERROR: " << surfRet << endl;
            return false;
        }

        vkInitData.surface = vk::SurfaceKHR { surface };

        //PHYSICAL DEVICE
        vk::PhysicalDeviceFeatures pdf {};
        pdf.samplerAnisotropy = true;
        vk::PhysicalDeviceVulkan13Features pdf13 {};
        pdf13.dynamicRendering = true;
        pdf13.synchronization2 = true;
        
        vkb::PhysicalDeviceSelector selector { vkInitData.bootInstance };
        selector.set_surface(surface);
        selector.set_required_features(pdf);
        selector.set_required_features_13(pdf13);
        selector.set_minimum_version(vkInitData.minVersionMajor, vkInitData.minVersionMinor);

        auto phyRet = selector.select();

        if(!phyRet) {
            cerr << "ERROR: " << phyRet.error().message() << endl;
            return false;
        }
        vkb::PhysicalDevice vkbPhysicalDevice = phyRet.value();
        vkInitData.physicalDevice = vk::PhysicalDevice { vkbPhysicalDevice.physical_device };

        //LOGICAL DEVICE
        vkb::DeviceBuilder devBuilder { vkbPhysicalDevice };
        auto devRet = devBuilder.build();
        if(!devRet) {
            cerr << "ERROR: " << devRet.error().message() << endl;
            return false;
        }
        vkb::Device vkbDevice = devRet.value();
        vkInitData.device = vk::Device { vkbDevice.device };
        vkInitData.bootDevice = vkbDevice;

        // SWAPCHAIN 
        if(!createVulkanSwapchain(vkInitData)) {
            return false;
        }

        // QUEUES
        if(!getVulkanQueue(vkInitData.bootDevice,
                            vkb::QueueType::graphics,
                            vkInitData.graphicsQueue)) {
         return false;
        }
        if(!getVulkanQueue(vkInitData.bootDevice,
                            vkb::QueueType::present,
                            vkInitData.presentQueue)) {
         return false;
        }

        return true;

    }

    void cleanupVulkanSetup(VulkanInitData &vkInitData) {
        
        //TODO
        cleanupVulkanSwapchain(vkInitData);
        
        vkInitData.device.destroy();
        vkInitData.instance.destroySurfaceKHR(vkInitData.surface);

        vkb::destroy_instance(vkInitData.bootInstance);
    }

    GLFWwindow* createGLFWWindow(string title, int width, int height, bool isResizable) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable);
    GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    return window;
}

    void cleanupGLFWWindow(GLFWwindow *window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

}
