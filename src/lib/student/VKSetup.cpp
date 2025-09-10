#include "student/VKSetup.hpp"

namespace student {

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

        return true;

    }

    void cleanupVulkanSetup(VulkanInitData &vkInitData) {
        
        //TODO
        vkInitData.instance.destroySurfaceKHR(vkInitData.surface);
        
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
