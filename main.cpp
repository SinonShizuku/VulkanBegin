#include "window/GlfwGeneral.hpp"
#include "myVulkan/VKBase.h"
#include "myVulkan/VKUtils.h"
#include "myVulkan/EasyVulkan.hpp"
#include "myVulkan/VKBase+.h"
#include <filesystem>
#include <string>

using namespace myVulkan;

// 渲染管线
pipelineLayout pipelineLayout_triangle;
pipeline pipeline_triangle;

//该函数调用easyVulkan::CreateRpwf_Screen()并存储返回的引用到静态变量，避免重复调用easyVulkan::CreateRpwf_Screen()
const auto& RenderPassAndFramebuffers() {
    static const auto& rpwf = easyVulkan::CreateRpwf_Screen();
    return rpwf;
}

void CreateLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayout_triangle.Create(pipelineLayoutCreateInfo);
}

//该函数用于创建管线
void CreatePipeline() {
    // 获取当前源文件的目录
    // 注意：__FILE__ 宏展开的路径可能取决于编译器和构建设置。
    std::filesystem::path sourceDir = std::filesystem::path(__FILE__).parent_path();

    // 基于源文件目录和原始相对路径 "../shader/" 构造着色器路径
    std::filesystem::path vertShaderPath = sourceDir / "shader/FirstTriangle.vert.spv";
    std::filesystem::path fragShaderPath = sourceDir / "shader/FirstTriangle.frag.spv";

    static shaderModule vert(vertShaderPath.c_str());
    static shaderModule frag(fragShaderPath.c_str());

    static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_triangle[2] = {
            vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
            frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    //创建管线
    auto Create = [] {
        graphicsPipelineCreateInfoPack pipelineCiPack;
        pipelineCiPack.createInfo.layout = pipelineLayout_triangle;
        pipelineCiPack.createInfo.renderPass = RenderPassAndFramebuffers().renderPass;
        pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
        pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
        pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineCiPack.colorBlendAttachmentStates.push_back({ .colorWriteMask = 0b1111 });
        pipelineCiPack.UpdateAllArrays();
        pipelineCiPack.createInfo.stageCount = 2;
        pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_triangle;
        pipeline_triangle.Create(pipelineCiPack);
    };
    auto Destroy = [] {
        pipeline_triangle.~pipeline();
    };
    //在回调函数中加入增删渲染管线
    graphicsBase::Base().AddCallback_CreateSwapchain(Create);
    graphicsBase::Base().AddCallback_DestroySwapchain(Destroy);
    //调用Create()以创建管线
    Create();
}

int main() {
    if (!InitializeWindow({1920,1080})) {
        std::cerr<<"Fatal:Initialize Window failed..."<<std::endl;
        return -1;
    }

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

    while(!glfwWindowShouldClose(pWindow))
    {
        // 当窗口最小化到任务栏时，阻塞运行
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();

        // 渲染流程
        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);

        commandBuffer_graphics.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        graphicsBase::Base().CmdTransferImageOwnership(commandBuffer_graphics);

        auto i = graphicsBase::Base().CurrentImageIndex();
        renderPass.CmdBegin(commandBuffer_graphics, framebuffers[i], {{}, windowSize }, clearColor);

        // 渲染命令部分
        vkCmdBindPipeline(commandBuffer_graphics, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_triangle);
        vkCmdDraw(commandBuffer_graphics, 3, 1, 0, 0);

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