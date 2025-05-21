#pragma once

#include "window/GlfwGeneral.hpp"
#include "myVulkan/VKBase.h"
#include "myVulkan/VKUtils.h"
#include "myVulkan/EasyVulkan.hpp"
#include "myVulkan/VKBase+.h"

#include "scene.h"


using namespace myVulkan;

// 当前目录
std::filesystem::path rootDir = std::filesystem::path(__FILE__).parent_path();

// 渲染管线
pipelineLayout pipelineLayout_triangle;
pipeline pipeline_triangle;

//描述符
descriptorSetLayout descriptorSetLayout_triangle;
descriptorSetLayout descriptorSetLayout_texture;
pipelineLayout pipelineLayout_texture;
pipeline pipeline_texture;

//该函数调用easyVulkan::CreateRpwf_Screen()并存储返回的引用到静态变量，避免重复调用easyVulkan::CreateRpwf_Screen()
const auto& RenderPassAndFramebuffers() {
    static const auto& rpwf = easyVulkan::CreateRpwf_Screen();
    return rpwf;
}

//创建管线布局
void CreateLayout() {
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding_texture = {
            .binding = 0,                                                //描述符被绑定到0号binding
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, //类型为带采样器的图像
            .descriptorCount = 1,                                        //个数是1个
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT                   //在片段着色器阶段采样图像
    };
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo_texture = {
            .bindingCount = 1,
            .pBindings = &descriptorSetLayoutBinding_texture
    };
    descriptorSetLayout_texture.Create(descriptorSetLayoutCreateInfo_texture);
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .setLayoutCount = 1,
            .pSetLayouts = descriptorSetLayout_texture.Address()
    };
    pipelineLayout_texture.Create(pipelineLayoutCreateInfo);
}

//该函数用于创建管线
void CreatePipeline() {
    static shaderModule vert((rootDir / "shader/Texture.vert.spv").c_str());
    static shaderModule frag((rootDir / "shader/Texture.frag.spv").c_str());
    static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_texture[2] = {
            vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
            frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    auto Create = [] {
        graphicsPipelineCreateInfoPack pipelineCiPack;
        pipelineCiPack.createInfo.layout = pipelineLayout_texture;
        pipelineCiPack.createInfo.renderPass = RenderPassAndFramebuffers().renderPass;
        pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(vertex2d), VK_VERTEX_INPUT_RATE_VERTEX);
        pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex2d, position));
        pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex2d, texCoord));
        pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
        pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
        pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineCiPack.colorBlendAttachmentStates.push_back({ .colorWriteMask = 0b1111 });
        pipelineCiPack.UpdateAllArrays();
        pipelineCiPack.createInfo.stageCount = 2;
        pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_texture;
        pipeline_texture.Create(pipelineCiPack);
    };
    auto Destroy = [] {
        pipeline_texture.~pipeline();
    };
    graphicsBase::Base().AddCallback_CreateSwapchain(Create);
    graphicsBase::Base().AddCallback_DestroySwapchain(Destroy);
    Create();
}


int main() {
    // 初始化窗口
    if (!InitializeWindow({1920,1080})) {
        std::cerr<<"Fatal:Initialize Window failed..."<<std::endl;
        return -1;
    }

    // 2d纹理贴图
    texture2d t((rootDir / "source/img.png").c_str(),VK_FORMAT_R8G8B8A8_UNORM,VK_FORMAT_R8G8B8A8_UNORM, true);

    VkSamplerCreateInfo samplerCreateInfo = texture::SamplerCreateInfo();
    sampler sampler(samplerCreateInfo);


//    easyVulkan::BootScreen((rootDir / "source/img.png").c_str(), VK_FORMAT_R8G8B8A8_UNORM);
//    std::this_thread::sleep_for(std::chrono::seconds(1));//需要#include <thread>

    // 定义缓冲帧缓冲
    const auto& [renderPass, framebuffers] = RenderPassAndFramebuffers();

    // 创建渲染管线
    CreateLayout();
    CreatePipeline();

    // 同步机制
    fence fence;
    semaphore semaphore_imageIsAvailable;
    semaphore semaphore_ownershipIsTransfered;

    // 命令缓冲
    commandBuffer commandBuffer_graphics;
    commandBuffer commandBuffer_presentation;
    commandPool commandPool_graphics(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool commandPool_presentation( graphicsBase::Base().QueueFamilyIndex_Presentation(),VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool_graphics.AllocateBuffers(commandBuffer_graphics);
    commandPool_presentation.AllocateBuffers(commandBuffer_presentation);

    // 清屏颜色
    VkClearValue clearColor = { .color = { 1.f, 1.f, 1.f, 1.f } };//红色

    vertex2d vertices2d[] = {
            { { -.5f, -.5f }, { 0, 0 } },
            { {  .5f, -.5f }, { 1, 0 } },
            { { -.5f,  .5f }, { 0, 1 } },
            { {  .5f,  .5f }, { 1, 1 } }
    };
    vertexBuffer vertexBuffer(sizeof vertices2d);
    vertexBuffer.TransferData(vertices2d);


    // 索引缓冲
    uint16_t indices[] = {
            0, 1, 2,
            1, 2, 3
    };
    indexBuffer indexBuffer(sizeof indices);
    indexBuffer.TransferData(indices);

    //// 创建并写入描述符
    VkDescriptorPoolSize descriptorPoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
    };
    descriptorPool descriptorPool(1, descriptorPoolSizes);
    descriptorSet descriptorSet_texture;
    descriptorPool.AllocateSets(descriptorSet_texture, descriptorSetLayout_texture);
    VkDescriptorImageInfo imageInfo = {
            .sampler = sampler,
            .imageView = t.ImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    descriptorSet_texture.Write(imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    while(!glfwWindowShouldClose(pWindow))
    {
        // 当窗口最小化到任务栏时，阻塞运行
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();

        // 渲染流程
        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);

        auto i = graphicsBase::Base().CurrentImageIndex();

        commandBuffer_graphics.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        graphicsBase::Base().CmdTransferImageOwnership(commandBuffer_graphics);
        renderPass.CmdBegin(commandBuffer_graphics, framebuffers[i], {{}, windowSize }, clearColor);

        // 渲染命令部分
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer_graphics, 0, 1, vertexBuffer.Address(), &offset);
        vkCmdBindPipeline(commandBuffer_graphics, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_texture);
        vkCmdBindDescriptorSets(commandBuffer_graphics, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_texture, 0, 1, descriptorSet_texture.Address(), 0, nullptr);
        vkCmdDraw(commandBuffer_graphics, 4, 1, 0, 0);

        renderPass.CmdEnd(commandBuffer_graphics);
        commandBuffer_graphics.End();

        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer_graphics, semaphore_imageIsAvailable);

        // 提交到呈现队列的命令缓冲区
        commandBuffer_presentation.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        graphicsBase::Base().CmdTransferImageOwnership(commandBuffer_presentation);
        commandBuffer_presentation.End();
        graphicsBase::Base().SubmitCommandBuffer_Presentation(commandBuffer_presentation, VK_NULL_HANDLE, semaphore_ownershipIsTransfered, fence);

        graphicsBase::Base().PresentImage(semaphore_ownershipIsTransfered);

        glfwPollEvents();
        TitleFPS();

        // 等待并重置fence
        fence.WaitAndReset();

    }
    TerminateWindow();
    std::cout<<"Finishing Window!"<<std::endl;
    return 0;
}