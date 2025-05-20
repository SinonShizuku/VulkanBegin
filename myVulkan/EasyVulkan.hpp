// 本文件为常用的渲染通道和帧缓冲的创建函数

#pragma once
#include "VKBase.h"
#include "VKUtils.h"
#include "VKBase+.h"



using namespace myVulkan;
const VkExtent2D& windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;

namespace easyVulkan {
    struct renderPassWithFramebuffers {
        myVulkan::renderPass renderPass;
        std::vector<framebuffer> framebuffers;
        ~renderPassWithFramebuffers() {
            for (auto& fb : framebuffers) {
                if (fb) {
                    fb.~framebuffer();
                }
            }
            framebuffers.clear();
        }
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
        // rpwf.framebuffers.resize(graphicsBase::Base().SwapchainImageCount());
        // VkFramebufferCreateInfo framebufferCreateInfo = {
        //         .renderPass = rpwf.renderPass,
        //         .attachmentCount = 1,
        //         .width = windowSize.width,
        //         .height = windowSize.height,
        //         .layers = 1
        // };
        // for (size_t i = 0; i < graphicsBase::Base().SwapchainImageCount(); i++) {
        //     VkImageView attachment = graphicsBase::Base().SwapchainImageView(i);
        //     framebufferCreateInfo.pAttachments = &attachment;
        //     rpwf.framebuffers[i].Create(framebufferCreateInfo);
        // }

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
        // graphicsBase::Base().AddCallback_DestroySwapchain(DestroyFramebuffers);

        return rpwf;
    }

    void BootScreen(const char* imagePath, VkFormat imageFormat) {
        //用先前写的函数从硬盘读图
        VkExtent2D imageExtent;
        std::unique_ptr<uint8_t[]> pImageData = texture::LoadFile(imagePath, imageExtent, FormatInfo(imageFormat));
        if (!pImageData)
            return;//这里我就偷个懒，不写错误信息了

        //图片暂存在缓冲
        stagingBuffer::BufferData_MainThread(pImageData.get(), FormatInfo(imageFormat).sizePerPixel * imageExtent.width * imageExtent.height);

        //创建同步对象
        semaphore semaphore_imageIsAvailable;
        fence fence;
        //分配命令缓冲区
        commandBuffer commandBuffer;
        graphicsBase::Plus().CommandPool_Graphics().AllocateBuffers(commandBuffer);

        //获取交换链图像
        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);

        //录制命令缓冲区
        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkExtent2D swapchainImageSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;
        bool blit =
                imageExtent.width != swapchainImageSize.width ||                       //宽
                imageExtent.height != swapchainImageSize.height ||                     //高
                imageFormat != graphicsBase::Base().SwapchainCreateInfo().imageFormat; //图像格式

        imageMemory imageMemory;
        if (blit) {
            VkImage image = stagingBuffer::AliasedImage2d_MainThread(imageFormat, imageExtent);
            if (image) {
                VkImageMemoryBarrier imageMemoryBarrier = {
                        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        nullptr,
                        0,
                        VK_ACCESS_TRANSFER_READ_BIT,
                        VK_IMAGE_LAYOUT_PREINITIALIZED,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_QUEUE_FAMILY_IGNORED,
                        VK_QUEUE_FAMILY_IGNORED,
                        image,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
                };
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                     0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
            else {
                VkImageCreateInfo imageCreateInfo = {
                        .imageType = VK_IMAGE_TYPE_2D,
                        .format = imageFormat,
                        .extent = { imageExtent.width, imageExtent.height, 1 },
                        .mipLevels = 1,
                        .arrayLayers = 1,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                };
                imageMemory.Create(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                VkBufferImageCopy region_copy = {
                        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                        .imageExtent = imageCreateInfo.extent
                };
                imageOperation::CmdCopyBufferToImage(commandBuffer,
                                                     stagingBuffer::Buffer_MainThread(),
                                                     imageMemory.Image(),
                                                     region_copy,
                                                     { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                                                     { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL });

                image = imageMemory.Image();
            }
            VkImageBlit region_blit = {
                    { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                    { {}, { int32_t(imageExtent.width), int32_t(imageExtent.height), 1 } },
                    { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                    { {}, { int32_t(swapchainImageSize.width), int32_t(swapchainImageSize.height), 1 } }
            };
            imageOperation::CmdBlitImage(commandBuffer,
                                         image,
                                         graphicsBase::Base().SwapchainImage(graphicsBase::Base().CurrentImageIndex()),
                                         region_blit,
                                         { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                                         { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }, VK_FILTER_LINEAR);
        }
        else {
            VkBufferImageCopy region_copy = {
                    .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
                    .imageExtent = { imageExtent.width, imageExtent.height, 1 }
            };
            imageOperation::CmdCopyBufferToImage(commandBuffer,
                                                 stagingBuffer::Buffer_MainThread(),
                                                 graphicsBase::Base().SwapchainImage(graphicsBase::Base().CurrentImageIndex()),
                                                 region_copy,
                                                 { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                                                 { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR  });
        }

        commandBuffer.End();

        //提交命令缓冲区
        VkPipelineStageFlags waitDstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo submitInfo = {
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = semaphore_imageIsAvailable.Address(),
                .pWaitDstStageMask = &waitDstStage,
                .commandBufferCount = 1,
                .pCommandBuffers = commandBuffer.Address()
        };
        graphicsBase::Base().SubmitCommandBuffer_Graphics(submitInfo, fence);
        //等待命令执行完毕
        fence.WaitAndReset();
        //呈现图像
        graphicsBase::Base().PresentImage();

        //别忘了释放命令缓冲区
        graphicsBase::Plus().CommandPool_Graphics().FreeBuffers(commandBuffer);
    }
}
