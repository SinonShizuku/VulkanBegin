#include "window/GlfwGeneral.hpp"
#include "myVulkan/VKBase.h"
#include "myVulkan/VKUtils.h"
#include "myVulkan/EasyVulkan.hpp"

using namespace myVulkan;

// 渲染管线
pipelineLayout pipelineLayout_triangle;
pipeline pipeline_triangle;

//该函数调用easyVulkan::CreateRpwf_Screen()并存储返回的引用到静态变量，避免重复调用easyVulkan::CreateRpwf_Screen()
const auto& RenderPassAndFramebuffers() {
    static const auto& rpwf = easyVulkan::CreateRpwf_Screen();
    return rpwf;
}
//该函数用于创建管线布局
void CreateLayout() {
    /*待后续填充*/
}
//该函数用于创建管线
void CreatePipeline() {
    /*待后续填充*/
}

int main() {
    if (!InitializeWindow({1920,1080})) {
        std::cerr<<"Fatal:Initialize Window failed..."<<std::endl;
        return -1;
    }

    // 定义缓冲帧缓冲
    const auto& [renderPass, framebuffers] = easyVulkan::CreateRpwf_Screen();

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
    VkClearValue clearColor = { .color = { 1.f, 0.f, 0.f, 1.f } };//红色

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
        // 渲染命令，待填充
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