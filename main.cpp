#include "window/GlfwGeneral.hpp"
#include "myVulkan/VKBase.h"
#include "myVulkan/VKUtils.h"

using namespace myVulkan;

int main() {
    if (!InitializeWindow({1920,1080})) {
        std::cerr<<"Fatal:Initialize Window failed..."<<std::endl;
        return -1;
    }

    fence fence(VK_FENCE_CREATE_SIGNALED_BIT); //以置位状态创建栅栏
    semaphore semaphore_imageIsAvailable;
    semaphore semaphore_renderingIsOver;

    commandBuffer commandBuffer;
    commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.AllocateBuffers(commandBuffer);

    while(!glfwWindowShouldClose(pWindow))
    {
        //等待并重置fence
        fence.WaitAndReset();

        //render
        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);

        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        //提交命令缓冲区
        //……
        commandBuffer.End();

        glfwPollEvents();
        TitleFPS();
    }
    TerminateWindow();
    std::cout<<"Finishing Window!"<<std::endl;
    return 0;
}