#include <iostream>
#include <string>
#include "student/VKSetup.hpp"
#include "student/VKCommand.hpp"
#include "student/VKImage.hpp"
#include "student/VKPipeline.hpp"
#include "student/VKMesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace student;

Assimp::Importer importer;

struct ForgeVertex {
    glm::vec3 pos;
    glm::vec4 color;
};

struct UniformPush {
    alignas(16) glm::mat4 modelMat;
};

glm::mat4 modelMat(1.0);
string transformString = "v";

float green = 0.0f;
float greenInc = 0.01f;

void printRM(string name, glm::mat4 &M) {
    cout << name << ":" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << M[j][i] << ", ";
        }
        cout << endl;
    }
}


void extractMeshData(aiMesh *mesh, HostMesh<ForgeVertex> &m) {
    m.vertices.clear();
    m.indices.clear();

    for(int i = 0; i < mesh->mNumVertices; i++) {
        aiVector3d verts = mesh->mVertices[i];
        float x = verts.x;
        float y = verts.y;
        float z = verts.z;
        glm::vec3 vertsVec = glm::vec3(x, y, z);


        ForgeVertex meshData = ForgeVertex{vertsVec, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)};

        m.vertices.push_back(meshData);
    }

    for(int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for(int j = 0; j < face.mNumIndices; j++) {
            m.indices.push_back(face.mIndices[j]);
        }

    }

}



bool leftMouseDown = false;

glm::vec2 lastMousePos = glm::vec2(0,0);

static void mouse_position_callback(
    GLFWwindow *window,
    double xpos,
    double ypos
) {
    glm::vec2 curPos(xpos, ypos);
    glm::vec2 relPos = curPos - lastMousePos;
    cout << "RELATIVE MOUSE: " << glm::to_string(relPos) << endl;


    //cout << "MOUSE: " << xpos << "," << ypos << endl;
    int width;
    int height;
    glfwGetFramebufferSize(window, &width, &height);

    relPos.x = relPos.x/(double)width;
    relPos.y = relPos.y/(double)height;

    //cout << "NORM MOUSE: " << xscale << "," << yscale << endl;
}

static void mouse_button_callback(
    GLFWwindow *window,
    int button,
    int action,
    int mods
) {
    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        if(action == GLFW_PRESS) {
            leftMouseDown = true;
            cout << "Left button press" << endl;
        }
        else if(action == GLFW_RELEASE){
            leftMouseDown = false;
            cout << "Left button release" << endl;
        }
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



void recordCommands (VulkanInitData &vkInitData, uint32_t indexFIF, uint32_t indexSwap,
                        vk::CommandBuffer &commandBuffer, 
                        vk::QueryPool &queryPool,
                        VulkanPipelineData &pipelineData,
                        vector<VulkanMesh> &allMeshes) {
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

    vk::RenderingInfoKHR renderInfo {};
    renderInfo.setRenderArea(vk::Rect2D{ {0,0}, vkInitData.swapchain.extent })
                .setLayerCount(1)
                .setColorAttachments(colorAttach);

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

    string appName = "Assign04";
    string windowTitle = "Assign04: saboa";
    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow *window = createGLFWWindow(windowTitle, windowWidth, windowHeight);

    
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_position_callback);

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    lastMousePos = glm::vec2(mx,my);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    

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
    )
    );
    pipeInfo.attribDesc.push_back(vk::VertexInputAttributeDescription(
        1, //location
        0, //binding
        vk::Format::eR32G32B32A32Sfloat, //format
        offsetof(ForgeVertex, color) //offset
    )
    );
    pipeInfo.renderInfo.colorAttachmentCount = 1;
    pipeInfo.renderInfo.pColorAttachmentFormats = &(vkInitData.swapchain.format);
    pipeInfo.renderInfo.depthAttachmentFormat = vk::Format::eD32Sfloat;
    //pipelineCreateInfo.renderInfo = renderInfo;

    pipeInfo.pushConstantRanges = {
        {vk::ShaderStageFlagBits::eVertex, 0, sizeof(UniformPush)}
    };



    VulkanPipelineData pipelineData = createBasicVulkanPipeline(vkInitData, pipeInfo);

    /*
    HostMesh<ForgeVertex> hostMesh;
    hostMesh.vertices = {
        {{0.0f, -0.3f, 0.5f}},
        {{-0.2f, -0.6f, 0.5f}},
        {{-0.4f, -0.6f, 0.5f}},
        {{-0.6f, -0.3f, 0.5f}},
        {{0.0f, 0.5f, 0.5f}},
        {{+0.6f, -0.3f, 0.5f}},
        {{+0.4f, -0.6f, 0.5f}},
        {{+0.2f, -0.6f, 0.5f}},

    };
    //this should be a heart shape
    hostMesh.indices = {0,2,1, 2,0,3, 4,3,0, 4,0,5, 6,0,5, 7,0,6};
    */

    string modelPath = "./sampleModels/sphere.obj"; //default path
    if(argc >= 2) {
        modelPath = argv[1];
    }

    const aiScene *scene = importer.ReadFile(modelPath,
        aiProcess_Triangulate | aiProcess_FlipUVs |
        aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cerr << "Error: " << importer.GetErrorString() << endl;
        exit(1);
    }


    bool useStaging = true;
    VulkanStagingData stagingData {};
    if(useStaging) {
        stagingData = beginStagingVulkanBufferCopies(
                        vkInitData, commandData.commandPool);
    }

    /*
    VulkanMesh mesh = createVulkanMesh(vkInitData, hostMesh, useStaging);

    copyToVulkanMesh(vkInitData, mesh, hostMesh, useStaging, stagingData);
    if(useStaging) {
        endStagingVulkanBufferCopies(vkInitData, commandData.commandPool, stagingData);
    }
    */

    vector<VulkanMesh> allMeshes {};

    for(int i = 0; i < scene->mNumMeshes; i++) {
        HostMesh<ForgeVertex> sceneMesh;
        extractMeshData(scene->mMeshes[i], sceneMesh);
        VulkanMesh mesh = createVulkanMesh(vkInitData, sceneMesh, useStaging);
        copyToVulkanMesh(vkInitData, mesh, sceneMesh, useStaging, stagingData);

        allMeshes.push_back(mesh);
    }
    if(useStaging) {
        endStagingVulkanBufferCopies(vkInitData, commandData.commandPool, stagingData);
    }

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t indexFIF = framesRendered % numberFramesInFlight;
        uint32_t indexSwap = prepareFrameInFlight(vkInitData, commandData, indexFIF);

        recordCommands(vkInitData, indexFIF, indexSwap, commandData.perFIF[indexFIF].commandBuffer,
                        queryPools[indexFIF], pipelineData, allMeshes);

        submitToGraphicsQueue(vkInitData, commandData, indexFIF, indexSwap);

        if(!presentSwapImage(vkInitData, commandData, indexFIF, indexSwap)) {
            recreateVulkanSwapchain(vkInitData);
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

    //cleanupVulkanMesh(vkInitData, mesh);
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