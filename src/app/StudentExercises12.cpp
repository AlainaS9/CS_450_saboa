#include <iostream>
#include <string>
#include "student/VKSetup.hpp"
#include "student/VKCommand.hpp"
#include "student/VKImage.hpp"
#include "student/VKPipeline.hpp"
#include "student/VKMesh.hpp"
#include "student/VKUniform.hpp"

using namespace std;
using namespace student;

struct ForgeVertex {
    glm::vec3 pos;
    glm::vec4 color;
};

struct UniformPush {
    alignas(16) glm::mat4 modelMat;
};

struct UBOVertex {
    alignas(16) glm::mat4 viewMat;
    alignas(16) glm::mat4 projMat;
};

UBOVertex uboVertHost {};

glm::mat4 modelMat(1.0);
string transformString = "v";

float green = 0.0f;
float greenInc = 0.01f;

void printRM(string name, glm::mat3 &m) {
    cout << name << ":" << endl;
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            cout << m[j][i] << "\t";
        }
        cout << endl;
    }
}

void printRM(string name, glm::mat4 &m) {
    cout << name << ":" << endl;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            cout << m[j][i] << "\t";
        }
        cout << endl;
    }
}

static void key_callback(GLFWwindow *window, int key, int scancode, int state, int mods) {
    if(state == GLFW_PRESS || state == GLFW_REPEAT) {
        if(key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, 1);
        }
        else if(key == GLFW_KEY_Q) {
            modelMat = glm::rotate(glm::radians(5.0f), glm::vec3(0,0,1))*modelMat;
            transformString = "R(5)*" + transformString;
        }
        else if(key == GLFW_KEY_E) {
            modelMat = glm::rotate(glm::radians(-5.0f), glm::vec3(0,0,1))*modelMat;
            transformString = "R(-5)*" + transformString;
        }
        else if(key == GLFW_KEY_F) {
            modelMat = glm::scale(glm::vec3(0.8f, 1.0f, 1.0f))*modelMat;
            transformString = "S(0.8, 1.0)*" + transformString;
        }
        else if(key == GLFW_KEY_G) {
            modelMat = glm::scale(glm::vec3(1.25f, 1.0f, 1.0f))*modelMat;
            transformString = "S(01.25, 1.0)*" + transformString;
        }
        else if(key == GLFW_KEY_R) {
            modelMat = glm::scale(glm::vec3(1.0f, 0.8f, 1.0f))*modelMat;
            transformString = "S(1.0, 0.8)*" + transformString;
        }
        else if(key == GLFW_KEY_T) {
            modelMat = glm::scale(glm::vec3(1.0f, 1.25f, 1.0f))*modelMat;
            transformString = "S(1.0, 1.25)*" + transformString;
        }
        else if(key == GLFW_KEY_W) {
            modelMat = glm::translate(glm::vec3(0.0f, 0.1f, 0.0f))*modelMat;
            transformString = "T(0.0, 0.1)*" + transformString;
        }
        else if(key == GLFW_KEY_S) {
            modelMat = glm::translate(glm::vec3(0.0f, -0.1f, 0.0f))*modelMat;
            transformString = "T(0.0, -0.1)*" + transformString;
        }
        else if(key == GLFW_KEY_A) {
            modelMat = glm::translate(glm::vec3(0.1f, 0.0f, 0.0f))*modelMat;
            transformString = "T(0.1, 0.0)*" + transformString;
        }
        else if(key == GLFW_KEY_D) {
            modelMat = glm::translate(glm::vec3(-0.1f, 0.0f, 0.0f))*modelMat;
            transformString = "T(-0.1, 0.0)*" + transformString;
        }
        else if(key == GLFW_KEY_SPACE) {
            modelMat = glm::mat4(1.0);
            transformString = "v";
        }
        cout << transformString << endl;
    }
}

void recordCommands (VulkanInitData &vkInitData, 
                        vector<VulkanImage> &allDepthImages,
                        uint32_t indexFIF, uint32_t indexSwap,
                        vk::CommandBuffer &commandBuffer, 
                        vk::QueryPool &queryPool,
                        VulkanPipelineData &pipelineData,
                        vector<VulkanMesh> &allMeshes,
                        UBOData &uboVertData,
                        vk::DescriptorSet &descriptorSet) {
    commandBuffer.begin(vk::CommandBufferBeginInfo());

    commandBuffer.resetQueryPool(queryPool, 0, 2);
    commandBuffer.writeTimestamp2(vk::PipelineStageFlagBits2::eTopOfPipe, 
                                    queryPool, 0);

    VulkanImageTransition colorBarrier = createVulkanImageTransition(
                                      vkInitData.swapchain.images[indexSwap],
                                      VK_IMAGE_TRANSITION_TYPE::UNDEF_TO_COLOR
    );
    performVulkanImageTransition(commandBuffer, colorBarrier);

    VulkanImageTransition presentBarrier = createVulkanImageTransition(
                                      vkInitData.swapchain.images[indexSwap],
                                      VK_IMAGE_TRANSITION_TYPE::COLOR_TO_PRESENT
    );
    performVulkanImageTransition(commandBuffer, presentBarrier);

    vk::RenderingAttachmentInfoKHR colorAttach{};
    colorAttach.setImageView(vkInitData.swapchain.views[indexSwap])
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.5f, 1.0f));

    /*
    green += greenInc;
    if(green > 1.0f) {
        green = 0.0f;
    }
    */

    vk::RenderingAttachmentInfoKHR depthAttach {};
    depthAttach.setImageView(allDepthImages[indexSwap].view)
                .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue(vk::ClearDepthStencilValue {1.0f, 0});

    vk::RenderingInfoKHR renderInfo {};
    renderInfo.setRenderArea(vk::Rect2D{ {0,0}, vkInitData.swapchain.extent })
                .setLayerCount(1)
                .setColorAttachments(colorAttach)
                .setPDepthAttachment(&depthAttach);

    commandBuffer.beginRendering(renderInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                    pipelineData.graphicsPipeline);
    
    vk::Viewport viewport(0,0,
                        (float)vkInitData.swapchain.extent.width,
                        (float)vkInitData.swapchain.extent.height,
                        0.0f, 1.0f);
    vk::Rect2D scissors({0,0}, vkInitData.swapchain.extent);

    commandBuffer.setViewport(0, {viewport});
    commandBuffer.setScissor(0, {scissors});

    UniformPush pc {};
    pc.modelMat = modelMat;

    commandBuffer.pushConstants(
        pipelineData.layout,
        vk::ShaderStageFlagBits::eVertex,
        0, sizeof(UniformPush),
        &pc
    );

    //uboVertHost.viewMat = glm::mat4(1.0);
    //uboVertHost.projMat = glm::mat4(1.0);

    glm::vec3 eye = glm::vec3(1,0,1);
    glm::vec3 center = glm::vec3(0,0,0);
    glm::vec3 up = glm::vec3(0,1,0);

    copyToHostVisibleVulkanBuffer(
        vkInitData,
        uboVertData.bufferData[indexFIF],
        &uboVertHost
    );

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineData.layout,
        0, descriptorSet, {}
    );

    for(int i = 0; i < allMeshes.size(); i++) {
        recordDrawVulkanMesh(commandBuffer, allMeshes.at(i));
    }

    commandBuffer.endRendering();
    
    commandBuffer.writeTimestamp2(vk::PipelineStageFlagBits2::eBottomOfPipe, queryPool, 1);

    commandBuffer.end();
}

const char* getDeviceTypeString(vk::PhysicalDeviceType t) {
    switch(t) {
        case vk::PhysicalDeviceType::eDiscreteGpu: return "Discrete GPU";
        case vk::PhysicalDeviceType::eIntegratedGpu: return "Integrated GPU";
        case vk::PhysicalDeviceType::eVirtualGpu: return "Virtual GPU";
        case vk::PhysicalDeviceType::eCpu: return "CPU";
        default: return "Other";
    }
}

void printPhysicalDeviceProperties(vk::PhysicalDevice &pd) {
    vk::PhysicalDeviceProperties props = pd.getProperties();
    uint32_t api = props.apiVersion;

    cout << "Name: " << props.deviceName.data() << endl;
    cout << "Type: " << getDeviceTypeString(props.deviceType) << endl;
    cout << "API Version: " << VK_VERSION_MAJOR(api) << "."
                                << VK_VERSION_MINOR(api) << "."
                                << VK_VERSION_PATCH(api) << endl;
}

void listAvailablePhysicalDevices(VulkanInitData &vkInitData) {
    vector<vk::PhysicalDevice> allDev = vkInitData.instance.enumeratePhysicalDevices();
    cout << "Found " << allDev.size() << " physical devices:" << endl;
    for(int i = 0; i < allDev.size(); i++) {
        cout << "**DEVICE " << i << "***********" << endl;
        printPhysicalDeviceProperties(allDev.at(i));
    }
}


int main(int argc, char **argv) {
    cout << "BEGIN EXERCISES!!!!" << endl;

    glm::mat3 M = glm::mat3(1,2,3,4,5,6,7,8,9);
    printRM("Simple", M);

    string appName = "StudentExercises12";
    string windowTitle = appName;
    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow *window = createGLFWWindow(windowTitle, windowWidth, windowHeight);

    glfwSetKeyCallback(window, key_callback);

    VulkanInitData vkInitData {};
    vkInitData.appName = appName;
    vkInitData.window = window;
    if(!createVulkanSetup(vkInitData)) {
        cleanupGLFWWindow(window);
        exit(1);
    }

    listAvailablePhysicalDevices(vkInitData);

    cout << "** Chosen Physical Device: ********" << endl;
    printPhysicalDeviceProperties(vkInitData.physicalDevice);

    uint32_t apiVersion = vk::enumerateInstanceVersion();
    cout << "Loader supports Vulkan " 
            << VK_VERSION_MAJOR(apiVersion) << "."
            << VK_VERSION_MINOR(apiVersion) << "."
            << VK_VERSION_PATCH(apiVersion) << endl;

    int numberFramesInFlight = 2;
    VulkanCommandData commandData = createVulkanCommandData(vkInitData, numberFramesInFlight);

    uint64_t framesRendered = 0;

    UBOData uboVertData = createVulkanUniformBufferData(
        vkInitData, sizeof(UBOVertex), numberFramesInFlight
    );

    vk::QueryPoolCreateInfo qpci {};
    qpci.queryType = vk::QueryType::eTimestamp;
    qpci.queryCount = 2;
    vector<vk::QueryPool> queryPools {};
    for(int i = 0; i < commandData.numberFramesInFlight; i++) {
        queryPools.push_back(vkInitData.device.createQueryPool(qpci));
    }

    VulkanPipelineCreationInfo pipeInfo {};
    pipeInfo.vertSPVFilename = "build/compiledshaders/" + appName + "/shader.vert.spv";
    pipeInfo.fragSPVFilename = "build/compiledshaders/" + appName + "/shader.frag.spv";
    pipeInfo.bindDesc = vk::VertexInputBindingDescription(
        0, sizeof(ForgeVertex), vk::VertexInputRate::eVertex
    );
    pipeInfo.attribDesc.push_back(vk::VertexInputAttributeDescription(
        0, //location
        0, //binding
        vk::Format::eR32G32B32Sfloat, //format
        offsetof(ForgeVertex, pos) //offset
    ));

    pipeInfo.attribDesc.push_back(vk::VertexInputAttributeDescription(
        1, //location
        0, //binding
        vk::Format::eR32G32B32A32Sfloat, //format
        offsetof(ForgeVertex, color) //offset
    ));

    pipeInfo.renderInfo.colorAttachmentCount = 1;
    pipeInfo.renderInfo.pColorAttachmentFormats = &(vkInitData.swapchain.format);
    pipeInfo.renderInfo.depthAttachmentFormat = vk::Format::eD32Sfloat;
    //pipelineCreateInfo.renderInfo = renderInfo;

    pipeInfo.pushConstantRanges.push_back(
        {vk::ShaderStageFlagBits::eVertex, 0, sizeof(UniformPush)});

    vector<vk::DescriptorSetLayoutBinding> allBindings = {
        vk::DescriptorSetLayoutBinding(
            0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex
        )
    };

    vk::DescriptorSetLayout layout = vkInitData.device.createDescriptorSetLayout(
        { {}, allBindings }
    );
    pipeInfo.allDescSetLayouts = { layout };
    

    VulkanPipelineData pipelineData = createBasicVulkanPipeline(vkInitData, pipeInfo);

    vector<vk::DescriptorPoolSize> poolSizes = {
        vk::DescriptorPoolSize(
            vk::DescriptorType::eUniformBuffer,
            numberFramesInFlight
        )
    };

    vk::DescriptorPool descPool = vkInitData.device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo()
            .setPoolSizes(poolSizes)
            .setMaxSets(numberFramesInFlight)
    );

    vector<vk::DescriptorSetLayout> frameLayouts;
    for(int i = 0; i < numberFramesInFlight; i++) {
        frameLayouts.push_back(pipelineData.allDescSetLayouts.at(0));
    }

    vector<vk::DescriptorSet> descSets = vkInitData.device.allocateDescriptorSets(
        vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(descPool)
            .setDescriptorSetCount(numberFramesInFlight)
            .setSetLayouts(frameLayouts)
    );

    for(int i = 0; i < numberFramesInFlight; i++) {
        vector<vk::WriteDescriptorSet> writes {};

        vk::DescriptorBufferInfo bufferVertInfo
        = vk::DescriptorBufferInfo()
            .setBuffer(uboVertData.bufferData[i].buffer)
            .setOffset(0)
            .setRange(sizeof(UBOVertex));

        vk::WriteDescriptorSet bufferVertWrite
        = vk::WriteDescriptorSet()
            .setDstSet(descSets[i])
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setBufferInfo(bufferVertInfo);

        writes.push_back(bufferVertWrite);
        vkInitData.device.updateDescriptorSets(writes, {});
    };

    vector<HostMesh<ForgeVertex>> allHostMeshes {};

    HostMesh<ForgeVertex> hostMesh;
    hostMesh.vertices = {
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{+0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{+0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{-0.5f, +0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}}
    };
    hostMesh.indices = {0,2,1, 2,0,3};
    allHostMeshes.push_back(hostMesh);

    
    HostMesh<ForgeVertex> hostMesh2;
    hostMesh2.vertices = {
        {{-0.1f, -0.25f, -0.7f}, {0.0f, 1.0f, 1.0f, 1.0f}},
        {{+0.8f, -0.25f, -0.7f}, {0.0f, 1.0f, 1.0f, 1.0f}},
        {{+0.8f, +0.25f, -0.7f}, {0.0f, 1.0f, 1.0f, 1.0f}},
        {{-0.1f, +0.25f, -0.7f}, {0.0f, 1.0f, 1.0f, 1.0f}}
    };
    hostMesh2.indices = {0,2,1, 2,0,3};
    allHostMeshes.push_back(hostMesh2);


    bool useStaging = true;
    VulkanStagingData stagingData {};

    vector<VulkanImage> allDepthImages {};
    recreateAllVulkanDepthImages(vkInitData, stagingData.commandBuffer, allDepthImages);

    if(useStaging) {
        stagingData = beginStagingVulkanBufferCopies(
                        vkInitData, commandData.commandPool);
    }

    vector<VulkanMesh> allMeshes {};
    for(int i = 0; i < allHostMeshes.size(); i++) {
        VulkanMesh mesh = createVulkanMesh(vkInitData, allHostMeshes.at(i), useStaging);
        copyToVulkanMesh(vkInitData, mesh, allHostMeshes.at(i), useStaging, stagingData);
        allMeshes.push_back(mesh);
    }

    VulkanMesh mesh = createVulkanMesh(vkInitData, hostMesh, useStaging);


    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t indexFIF = framesRendered % numberFramesInFlight;
        uint32_t indexSwap = prepareFrameInFlight(vkInitData, commandData, indexFIF);

        recordCommands(vkInitData, allDepthImages, indexFIF, indexSwap, commandData.perFIF[indexFIF].commandBuffer,
                        queryPools[indexFIF], pipelineData, allMeshes, uboVertData, descSets[indexFIF]);

        submitToGraphicsQueue(vkInitData, commandData, indexFIF, indexSwap);

        if(!presentSwapImage(vkInitData, commandData, indexFIF, indexSwap)) {
            recreateVulkanSwapchain(vkInitData);
            VulkanStagingData depthStage = beginStagingVulkanBufferCopies(
                vkInitData, commandData.commandPool
            );
            recreateAllVulkanDepthImages(
                vkInitData, depthStage.commandBuffer, allDepthImages);
            endStagingVulkanBufferCopies(
                vkInitData, commandData.commandPool, depthStage);
        }

        framesRendered++;

        uint64_t timeStamps[2] = {};
        vkInitData.device.getQueryPoolResults(queryPools[indexFIF], 0, 2,
                            sizeof(timeStamps), timeStamps, sizeof(uint64_t),
                            vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);

        auto props = vkInitData.physicalDevice.getProperties();
        double nsPerTick = props.limits.timestampPeriod;
        double deltaNs = (timeStamps[1] - timeStamps[0])*nsPerTick;

      //  cout << "TIME PER FIF " << indexFIF << ": " << deltaNs << endl;
        }

    vkInitData.device.waitIdle();

    vkInitData.device.destroyDescriptorPool(descPool);

    cleanupVulkanUniformBufferData(vkInitData, uboVertData);

    cleanupAllVulkanDepthImages(vkInitData, allDepthImages);

    for(int i = 0; i < allMeshes.size(); i++) {
        cleanupVulkanMesh(vkInitData, allMeshes.at(i));
    }
    allMeshes.clear();

    cleanupVulkanPipeline(vkInitData, pipelineData);

    for(int i = 0; i < queryPools.size(); i++) {
        vkInitData.device.destroyQueryPool(queryPools[i]);
    }
    queryPools.clear();

    cleanupVulkanCommandData(vkInitData, commandData);
    cleanupVulkanSetup(vkInitData);
    cleanupGLFWWindow(window);

    return 0;
}