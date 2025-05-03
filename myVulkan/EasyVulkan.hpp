// 本文件为常用的渲染通道和帧缓冲的创建函数

#pragma once
#include "VKBase.h"
#include "VKUtils.h"

using namespace myVulkan;
const VkExtent2D& windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;

namespace easyVulkan {
    using namespace myVulkan;
    struct renderPassWithFramebuffers {
        myVulkan::renderPass renderPass;
        std::vector<framebuffer> framebuffers;
    };

    //创建一个最简单的渲染通道：直接渲染到交换链图像，且不做深度测试等任何测试的渲染通道
    const auto& CreateRpwf_Screen() {
        static renderPassWithFramebuffers rpwf;

        //描述图像附件（交换链图像）
        VkAttachmentDescription attachmentDescription = {
                .format = graphicsBase::Base().SwapchainCreateInfo().imageFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        //设置渲染管线中的颜色附件引用
        VkAttachmentReference attachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        //描述子通道
        VkSubpassDescription subpassDescription = {
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &attachmentReference
        };

        //渲染通道开始时子通道的隐式依赖
        VkSubpassDependency subpassDependency = {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        };

        //创建渲染通道
        VkRenderPassCreateInfo renderPassCreateInfo = {
                .attachmentCount = 1,
                .pAttachments = &attachmentDescription,
                .subpassCount = 1,
                .pSubpasses = &subpassDescription,
                .dependencyCount = 1,
                .pDependencies = &subpassDependency
        };
        rpwf.renderPass.Create(renderPassCreateInfo);

        //创建帧缓冲
        rpwf.framebuffers.resize(graphicsBase::Base().SwapchainImageCount());
        VkFramebufferCreateInfo framebufferCreateInfo = {
                .renderPass = rpwf.renderPass,
                .attachmentCount = 1,
                .width = windowSize.width,
                .height = windowSize.height,
                .layers = 1
        };
        for (size_t i = 0; i < graphicsBase::Base().SwapchainImageCount(); i++) {
            VkImageView attachment = graphicsBase::Base().SwapchainImageView(i);
            framebufferCreateInfo.pAttachments = &attachment;
            rpwf.framebuffers[i].Create(framebufferCreateInfo);
        }

        //定义创建和销毁帧缓冲的回调函数
        auto CreateFramebuffers = [] {
            rpwf.framebuffers.resize(graphicsBase::Base().SwapchainImageCount());
            VkFramebufferCreateInfo framebufferCreateInfo = {
                    .renderPass = rpwf.renderPass,
                    .attachmentCount = 1,
                    .width = windowSize.width,
                    .height = windowSize.height,
                    .layers = 1
            };
            for (size_t i = 0; i < graphicsBase::Base().SwapchainImageCount(); i++) {
                VkImageView attachment = graphicsBase::Base().SwapchainImageView(i);
                framebufferCreateInfo.pAttachments = &attachment;
                rpwf.framebuffers[i].Create(framebufferCreateInfo);
            }
        };
        auto DestroyFramebuffers = [] {
            rpwf.framebuffers.clear();//清空vector中的元素时会逐一执行析构函数
        };

        CreateFramebuffers();

        ExecuteOnce(rpwf); //防止再次调用本函数时，重复添加回调函数
        graphicsBase::Base().AddCallback_CreateSwapchain(CreateFramebuffers);
        graphicsBase::Base().AddCallback_DestroySwapchain(DestroyFramebuffers);

        return rpwf;
    }
}
