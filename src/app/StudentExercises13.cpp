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
    glm::vec3 pos = glm::vec3(0,0,0);
    glm::vec4 color = glm::vec4(1,1,1,1);
    glm::vec3 normal = glm::vec3(0,0,0);

    ForgeVertex() {};
    ForgeVertex(glm::vec3 p) {
        pos = p;
    }
    ForgeVertex(glm::vec3 p , glm::vec4 c) {
        pos = p;
        color = c;
    };
};

struct UniformPush {
    alignas(16) glm::mat4 modelMat;
    alignas(16) glm::mat4 normMat;
};

struct UBOVertex {
    alignas(16) glm::mat4 viewMat;
    alignas(16) glm::mat4 projMat;
};

UBOVertex uboVertHost {};

struct PointLight {
    alignas(16) glm::vec4 pos;
    alignas(16) glm::vec4 vpos;
    alignas(16) glm::vec4 color;
};

struct UBOFragment {
    PointLight light;
};

UBOFragment uboFragHost {};

glm::mat4 modelMat(1.0);
string transformString = "v";

float green = 0.0f;
float greenInc = 0.01f;

glm::vec3 eye = glm::vec3(1,0,1);

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

bool leftMouseDown = false;
glm::vec2 lastMousePos = glm::vec2(0,0);

void computeOneNormal(
    vector<ForgeVertex> &vertices,
    int i1,
    int i2,
    int i3) {
        
        glm::vec3 A = vertices[i1].pos;
        glm::vec3 B = vertices[i2].pos;
        glm::vec3 C = vertices[i3].pos;

        glm::vec3 S1 = B - A;
        glm::vec3 S2 = C - A;
        glm::vec3 N = glm::normalize(glm::cross(S1, S2));

        vertices[i1].normal += N;
        vertices[i2].normal += N;
        vertices[i3].normal += N;
    }

void computeAllNormals(
    vector<ForgeVertex> &vertices,
    vector<unsigned int> &indices) {

        for(int i = 0; i < indices.size(); i += 3) {
            computeOneNormal(
                vertices,
                indices[i],
                indices[i+1],
                indices[i+2]
            );
        }

        for(int i = 0; i < vertices.size(); i++) {
            vertices[i].normal = glm::normalize(vertices[i].normal);
        }
    }

void makeCylinder(
    HostMesh<ForgeVertex> &mesh,
    float length,
    float radius,
    float faceCnt
) {
    float angleInc = glm::radians(360.0/(float)faceCnt);
    float halfLen = length/2.0;
    mesh.vertices.clear();
    mesh.indices.clear();
    // Vertices
    for(int i = 0; i < faceCnt; i++) {
        //Two vertices
        float angle = angleInc*i;
        float x = halfLen;
        float y = radius * glm::sin(angle);
        float z = radius * glm::cos(angle);

        glm::vec3 left = glm::vec3(-x, y, z);
        glm::vec3 right = glm::vec3(x, y, z);

        ForgeVertex vleft = ForgeVertex(left, glm::vec4(1,0,0,1));
        ForgeVertex vright = ForgeVertex(right, glm::vec4(0,1,0,1));

        mesh.vertices.push_back(vleft);
        mesh.vertices.push_back(vright);
    }

    // Indices
    int vcnt = mesh.vertices.size();
    for(int i = 0; i < faceCnt; i++) {
        int i0 = i*2;
        int i1 = i0 + 1;
        int i2 = (i0 + 2)%vcnt;
        int i3 = (i0 + 3)%vcnt;

        mesh.indices.push_back(i0);
        mesh.indices.push_back(i1);
        mesh.indices.push_back(i2);

        mesh.indices.push_back(i1);
        mesh.indices.push_back(i3);
        mesh.indices.push_back(i2);
    }

    computeAllNormals(mesh.vertices, mesh.indices);
}


static void mouse_position_callback(
    GLFWwindow *window,
    double xpos,
    double ypos
) {
    glm::vec2 curPos(xpos, ypos);
    glm::vec2 relPos = curPos - lastMousePos;
    //cout << "RELATIVE MOUSE: " << glm::to_string(relPos) << endl;


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

void recordCommands (VulkanInitData &vkInitData, 
                        vector<VulkanImage> &allDepthImages,
                        uint32_t indexFIF, uint32_t indexSwap,
                        vk::CommandBuffer &commandBuffer, 
                        vk::QueryPool &queryPool,
                        VulkanPipelineData &pipelineData,
                        vector<VulkanMesh> &allMeshes,
                        UBOData &uboVertData,
                        UBOData &uboFragData,
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

    //uboVertHost.viewMat = glm::mat4(1.0);
    //uboVertHost.projMat = glm::mat4(1.0);
    if(leftMouseDown) {
        glm::vec4 eye4 = glm::vec4(eye, 1.0f);
        eye4 = glm::rotate(glm::radians(1.0f), glm::vec3(0,1,0))*eye4;
        eye = glm::vec3(eye4);
    }

    glm::vec3 center = glm::vec3(0,0,0);
    glm::vec3 up = glm::vec3(0,1,0);

    uboVertHost.viewMat = glm::lookAt(eye, center, up);

    float fovAngle = glm::radians(90.0f);
    float width = vkInitData.swapchain.extent.width;
    float height = vkInitData.swapchain.extent.height;
    float aspectRatio = width/height;
    float near = 0.01f;
    float far = 100.0f;
    uboVertHost.projMat = glm::perspective(fovAngle, aspectRatio, near, far);
    uboVertHost.projMat[1][1] *= -1.0f;

    copyToHostVisibleVulkanBuffer(
        vkInitData,
        uboVertData.bufferData[indexFIF],
        &uboVertHost
    );

    uboFragHost.light.pos = glm::rotate(glm::radians(1.0f), glm::vec3(0.0,1.0,0.0))*uboFragHost.light.pos;

    uboFragHost.light.vpos = uboVertHost.viewMat*uboFragHost.light.pos;

    copyToHostVisibleVulkanBuffer(
        vkInitData,
        uboFragData.bufferData[indexFIF],
        &uboFragHost
    );

    UniformPush pc {};
    pc.modelMat = modelMat;
    pc.normMat = glm::mat4(glm::transpose(glm::inverse(glm::mat3(uboVertHost.viewMat*modelMat))));

    commandBuffer.pushConstants(
        pipelineData.layout,
        vk::ShaderStageFlagBits::eVertex,
        0, sizeof(UniformPush),
        &pc
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

    string appName = "StudentExercises13";
    string windowTitle = appName;
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

    UBOData uboVertData = createVulkanUniformBufferData(
        vkInitData, sizeof(UBOVertex), numberFramesInFlight
    );

    uboFragHost.light.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    uboFragHost.light.pos = glm::vec4(0.0f, 0.5f, 0.5f, 0.1f);

    UBOData uboFragData = createVulkanUniformBufferData(
        vkInitData, sizeof(UBOFragment), numberFramesInFlight
    );

    cout << "test" << endl;

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

    pipeInfo.attribDesc.push_back(vk::VertexInputAttributeDescription(
        2, //location
        0, //binding
        vk::Format::eR32G32B32Sfloat, //format
        offsetof(ForgeVertex, normal) //offset
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
        ),
        vk::DescriptorSetLayoutBinding(
            1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex
        )
    };

    vk::DescriptorSetLayout layout = vkInitData.device.createDescriptorSetLayout(
        { {}, allBindings }
    );
    pipeInfo.allDescSetLayouts = { layout };

    cout << "test2" << endl;
    

    VulkanPipelineData pipelineData = createBasicVulkanPipeline(vkInitData, pipeInfo);

    int uboCnt = 2;
    vector<vk::DescriptorPoolSize> poolSizes = {
        vk::DescriptorPoolSize(
            vk::DescriptorType::eUniformBuffer,
            uboCnt*numberFramesInFlight
        )
    };

    cout << "descpoolsize" << endl;

    vk::DescriptorPool descPool = vkInitData.device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo()
            .setPoolSizes(poolSizes)
            .setMaxSets(numberFramesInFlight)
    );

    cout << "descpool" << endl;

    vector<vk::DescriptorSetLayout> frameLayouts;
    for(unsigned int i = 0; i < numberFramesInFlight; i++) {
        frameLayouts.push_back(pipelineData.allDescSetLayouts.at(0));
    }

    cout << "framelayouts" << endl;

    vector<vk::DescriptorSet> descSets = vkInitData.device.allocateDescriptorSets(
        vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(descPool)
            .setDescriptorSetCount(numberFramesInFlight)
            .setSetLayouts(frameLayouts)
    );

    cout << "descsets" << endl;

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


        vk::DescriptorBufferInfo bufferFragInfo
        = vk::DescriptorBufferInfo()
            .setBuffer(uboFragData.bufferData[i].buffer)
            .setOffset(0)
            .setRange(sizeof(UBOFragment));

        vk::WriteDescriptorSet bufferFragWrite
        = vk::WriteDescriptorSet()
            .setDstSet(descSets[i])
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setBufferInfo(bufferFragInfo);

        writes.push_back(bufferFragWrite);

        vkInitData.device.updateDescriptorSets(writes, {});
    };
    cout << "test3" << endl;

    vector<HostMesh<ForgeVertex>> allHostMeshes {};
    HostMesh<ForgeVertex> hostMesh;
    /*
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
    */

    HostMesh<ForgeVertex> cylinder {};
    makeCylinder(cylinder, 1.0, 0.5, 10);
    allHostMeshes.push_back(cylinder);

    cout << "hostmesh" << endl;

    bool useStaging = true;
    VulkanStagingData stagingData {};
cout << "stagingdata" << endl;
    vector<VulkanImage> allDepthImages {};
    cout << "ok" << endl;
    recreateAllVulkanDepthImages(vkInitData, stagingData.commandBuffer, allDepthImages);

    cout << "recreateall.." << endl;

    if(useStaging) {
        stagingData = beginStagingVulkanBufferCopies(
                        vkInitData, commandData.commandPool);
    }

    cout << "staging" << endl;

    vector<VulkanMesh> allMeshes {};
    for(int i = 0; i < allHostMeshes.size(); i++) {
        VulkanMesh mesh = createVulkanMesh(vkInitData, allHostMeshes.at(i), useStaging);
        copyToVulkanMesh(vkInitData, mesh, allHostMeshes.at(i), useStaging, stagingData);
        allMeshes.push_back(mesh);
    }

    VulkanMesh mesh = createVulkanMesh(vkInitData, hostMesh, useStaging);

cout << "test3.5" << endl;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t indexFIF = framesRendered % numberFramesInFlight;
        uint32_t indexSwap = prepareFrameInFlight(vkInitData, commandData, indexFIF);

        recordCommands(vkInitData, allDepthImages, indexFIF, indexSwap, commandData.perFIF[indexFIF].commandBuffer,
                        queryPools[indexFIF], pipelineData, allMeshes, uboVertData, uboFragData, descSets[indexFIF]);

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
      cout << "test4" << endl;
        }

    vkInitData.device.waitIdle();

    vkInitData.device.destroyDescriptorPool(descPool);

    cleanupVulkanUniformBufferData(vkInitData, uboFragData);
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