//
// Created by Jhong on 2025/5/1.
//

#pragma once
//#include <vulkan/vulkan_format_traits.hpp>
#include "VKFormat.h"
#include "EasyVKStart.h"
#include "VKBase.hpp"
#include "VKUtils.h"

namespace myVulkan{

    // 对渲染管线创建信息进行封装
    struct graphicsPipelineCreateInfoPack {
        VkGraphicsPipelineCreateInfo createInfo =
                { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        //Vertex Input
        VkPipelineVertexInputStateCreateInfo vertexInputStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        std::vector<VkVertexInputBindingDescription> vertexInputBindings;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
        //Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        //Tessellation
        VkPipelineTessellationStateCreateInfo tessellationStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
        //Viewport
        VkPipelineViewportStateCreateInfo viewportStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
        uint32_t dynamicViewportCount = 1;//动态视口/剪裁不会用到上述的vector，因此动态视口和剪裁的个数向这俩变量手动指定
        uint32_t dynamicScissorCount = 1;
        //Rasterization
        VkPipelineRasterizationStateCreateInfo rasterizationStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        //Multisample
        VkPipelineMultisampleStateCreateInfo multisampleStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        //Depth & Stencil
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        //Color Blend
        VkPipelineColorBlendStateCreateInfo colorBlendStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
        //Dynamic
        VkPipelineDynamicStateCreateInfo dynamicStateCi =
                { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        std::vector<VkDynamicState> dynamicStates;
        //--------------------
        graphicsPipelineCreateInfoPack() {
            SetCreateInfos();
            //若非派生管线，createInfo.basePipelineIndex不得为0，设置为-1
            createInfo.basePipelineIndex = -1;
        }
        //移动构造器，所有指针都要重新赋值
        graphicsPipelineCreateInfoPack(const graphicsPipelineCreateInfoPack& other) noexcept {
            createInfo = other.createInfo;
            SetCreateInfos();

            vertexInputStateCi = other.vertexInputStateCi;
            inputAssemblyStateCi = other.inputAssemblyStateCi;
            tessellationStateCi = other.tessellationStateCi;
            viewportStateCi = other.viewportStateCi;
            rasterizationStateCi = other.rasterizationStateCi;
            multisampleStateCi = other.multisampleStateCi;
            depthStencilStateCi = other.depthStencilStateCi;
            colorBlendStateCi = other.colorBlendStateCi;
            dynamicStateCi = other.dynamicStateCi;

            shaderStages = other.shaderStages;
            vertexInputBindings = other.vertexInputBindings;
            vertexInputAttributes = other.vertexInputAttributes;
            viewports = other.viewports;
            scissors = other.scissors;
            colorBlendAttachmentStates = other.colorBlendAttachmentStates;
            dynamicStates = other.dynamicStates;
            UpdateAllArrayAddresses();
        }
        //Getter，这里我没用const修饰符
        operator VkGraphicsPipelineCreateInfo& () { return createInfo; }
        //Non-const Function
        //该函数用于将各个vector中数据的地址赋值给各个创建信息中相应成员，并相应改变各个count
        void UpdateAllArrays() {
            createInfo.stageCount = shaderStages.size();
            vertexInputStateCi.vertexBindingDescriptionCount = vertexInputBindings.size();
            vertexInputStateCi.vertexAttributeDescriptionCount = vertexInputAttributes.size();
            viewportStateCi.viewportCount = viewports.size() ? uint32_t(viewports.size()) : dynamicViewportCount;
            viewportStateCi.scissorCount = scissors.size() ? uint32_t(scissors.size()) : dynamicScissorCount;
            colorBlendStateCi.attachmentCount = colorBlendAttachmentStates.size();
            dynamicStateCi.dynamicStateCount = dynamicStates.size();
            UpdateAllArrayAddresses();
        }
    private:
        //该函数用于将创建信息的地址赋值给basePipelineIndex中相应成员
        void SetCreateInfos() {
            createInfo.pVertexInputState = &vertexInputStateCi;
            createInfo.pInputAssemblyState = &inputAssemblyStateCi;
            createInfo.pTessellationState = &tessellationStateCi;
            createInfo.pViewportState = &viewportStateCi;
            createInfo.pRasterizationState = &rasterizationStateCi;
            createInfo.pMultisampleState = &multisampleStateCi;
            createInfo.pDepthStencilState = &depthStencilStateCi;
            createInfo.pColorBlendState = &colorBlendStateCi;
            createInfo.pDynamicState = &dynamicStateCi;
        }
        //该函数用于将各个vector中数据的地址赋值给各个创建信息中相应成员，但不改变各个count
        void UpdateAllArrayAddresses() {
            createInfo.pStages = shaderStages.data();
            vertexInputStateCi.pVertexBindingDescriptions = vertexInputBindings.data();
            vertexInputStateCi.pVertexAttributeDescriptions = vertexInputAttributes.data();
            viewportStateCi.pViewports = viewports.data();
            viewportStateCi.pScissors = scissors.data();
            colorBlendStateCi.pAttachments = colorBlendAttachmentStates.data();
            dynamicStateCi.pDynamicStates = dynamicStates.data();
        }
    };

    // 加类，为graphicsBase做补充
    class graphicsBasePlus {
        //格式属性
        VkFormatProperties formatProperties[std::size(formatInfos_v1_0)] = {};

        commandPool commandPool_graphics;
        commandPool commandPool_presentation;
        commandPool commandPool_compute;
        commandBuffer commandBuffer_transfer;//从commandPool_graphics分配
        commandBuffer commandBuffer_presentation;
        //静态变量
        static graphicsBasePlus singleton;
        //--------------------
        graphicsBasePlus() {
            //在创建逻辑设备时执行Initialize()
            auto Initialize = [] {
                if (graphicsBase::Base().QueueFamilyIndex_Graphics() != VK_QUEUE_FAMILY_IGNORED)
                    singleton.commandPool_graphics.Create(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
                            singleton.commandPool_graphics.AllocateBuffers(singleton.commandBuffer_transfer);
                if (graphicsBase::Base().QueueFamilyIndex_Compute() != VK_QUEUE_FAMILY_IGNORED)
                    singleton.commandPool_compute.Create(graphicsBase::Base().QueueFamilyIndex_Compute(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
                if (graphicsBase::Base().QueueFamilyIndex_Presentation() != VK_QUEUE_FAMILY_IGNORED &&
                    graphicsBase::Base().QueueFamilyIndex_Presentation() != graphicsBase::Base().QueueFamilyIndex_Graphics() &&
                    graphicsBase::Base().SwapchainCreateInfo().imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)
                    singleton.commandPool_presentation.Create(graphicsBase::Base().QueueFamilyIndex_Presentation(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
                            singleton.commandPool_presentation.AllocateBuffers(singleton.commandBuffer_presentation);
                for (size_t i = 0; i < std::size(singleton.formatProperties); i++)
                    vkGetPhysicalDeviceFormatProperties(graphicsBase::Base().PhysicalDevice(), VkFormat(i), &singleton.formatProperties[i]);
            };
            //在销毁逻辑设备时执行CleanUp()
            //如果你不需要更换物理设备或在运行中重启Vulkan（皆涉及重建逻辑设备），那么此CleanUp回调非必要
            //程序运行结束时，无论是否有这个回调，graphicsBasePlus中的对象必会在析构graphicsBase前被析构掉
            auto CleanUp = [] {
                singleton.commandPool_graphics.~commandPool();
                singleton.commandPool_presentation.~commandPool();
                singleton.commandPool_compute.~commandPool();
            };
            graphicsBase::Plus(singleton);
            graphicsBase::Base().AddCallback_CreateDevice(Initialize);
            graphicsBase::Base().AddCallback_DestroyDevice(CleanUp);
        }
        graphicsBasePlus(graphicsBasePlus&&) = delete;
        ~graphicsBasePlus() = default;
    public:
        //Getter
        const VkFormatProperties& FormatProperties(VkFormat format) const {
            if constexpr (ENABLE_DEBUG_MESSENGER){
                if (uint32_t(format) >= std::size(formatInfos_v1_0))
                    outStream << std::format("[ FormatProperties ] ERROR\nThis function only supports definite formats provided by VK_VERSION_1_0.\n"),
                            abort();
            }
            return formatProperties[format];
        }
        const commandPool& CommandPool_Graphics() const { return commandPool_graphics; }
        const commandPool& CommandPool_Compute() const { return commandPool_compute; }
        const commandBuffer& CommandBuffer_Transfer() const { return commandBuffer_transfer; }
        //Const Function
        //简化命令提交
        result_t ExecuteCommandBuffer_Graphics(VkCommandBuffer commandBuffer) const {
            fence fence;
            VkSubmitInfo submitInfo = {
                    .commandBufferCount = 1,
                    .pCommandBuffers = &commandBuffer
            };
            VkResult result = graphicsBase::Base().SubmitCommandBuffer_Graphics(submitInfo, fence);
            if (!result)
                fence.Wait();
            return result;
        }
        //该函数专用于向呈现队列提交用于接收交换链图像的队列族所有权的命令缓冲区
        result_t AcquireImageOwnership_Presentation(VkSemaphore semaphore_renderingIsOver, VkSemaphore semaphore_ownershipIsTransfered, VkFence fence = VK_NULL_HANDLE) const {
            if (VkResult result = commandBuffer_presentation.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
                return result;
            graphicsBase::Base().CmdTransferImageOwnership(commandBuffer_presentation);
            if (VkResult result = commandBuffer_presentation.End())
                return result;
            return graphicsBase::Base().SubmitCommandBuffer_Presentation(commandBuffer_presentation, semaphore_renderingIsOver, semaphore_ownershipIsTransfered, fence);
        }
    };
    inline graphicsBasePlus graphicsBasePlus::singleton;

    constexpr formatInfo FormatInfo(VkFormat format) {

        if constexpr (ENABLE_DEBUG_MESSENGER)
            if (uint32_t(format) >= std::size(formatInfos_v1_0))
                outStream << std::format("[ FormatInfo ] ERROR\nThis function only supports definite formats provided by VK_VERSION_1_0.\n"),
                        abort();

        return formatInfos_v1_0[uint32_t(format)];
    }
    constexpr VkFormat Corresponding16BitFloatFormat(VkFormat format_32BitFloat) {
        switch (format_32BitFloat) {
            case VK_FORMAT_R32_SFLOAT:
                return VK_FORMAT_R16_SFLOAT;
            case VK_FORMAT_R32G32_SFLOAT:
                return VK_FORMAT_R16G16_SFLOAT;
            case VK_FORMAT_R32G32B32_SFLOAT:
                return VK_FORMAT_R16G16B16_SFLOAT;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
        }
        return format_32BitFloat;
    }
    inline const VkFormatProperties& FormatProperties(VkFormat format) {
        return graphicsBase::Plus().FormatProperties(format);
    }

    //暂存缓冲区
    class stagingBuffer {
        class Holder {
            static stagingBuffer* Create() {
                static stagingBuffer stagingBuffer;
                graphicsBase::Base().AddCallback_DestroyDevice([] { stagingBuffer.~stagingBuffer(); });
                return &stagingBuffer;
            }
            stagingBuffer* pointer;
        public:
            Holder() : pointer(Create()) {}
            stagingBuffer& Get() const { return *pointer; }
        };
        static inline Holder stagingBuffer_mainThread; // 静态成员实例

    protected:
        class myVulkan::bufferMemory bufferMemory;
        VkDeviceSize memoryUsage = 0;
        image aliasedImage;
    public:
        stagingBuffer() = default;
        stagingBuffer(VkDeviceSize size) {
            Expand(size);
        }
        //Getter
        operator VkBuffer() const { return bufferMemory.Buffer(); }
        const VkBuffer* Address() const { return bufferMemory.AddressOfBuffer(); }
        VkDeviceSize AllocationSize() const { return bufferMemory.AllocationSize(); }
        VkImage AliasedImage() const { return aliasedImage; }
        //Const Function
        void RetrieveData(void* pData_src, VkDeviceSize size) const {
            bufferMemory.RetrieveData(pData_src, size);
        }
        //Non-const Function
        void Expand(VkDeviceSize size) {
            if (size <= AllocationSize())
                return;
            Release();
            VkBufferCreateInfo bufferCreateInfo = {
                    .size = size,
                    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            };
            bufferMemory.Create(bufferCreateInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        }
        void Release() {
            bufferMemory.~bufferMemory();
        }
        void* MapMemory(VkDeviceSize size) {
            Expand(size);
            void* pData_dst = nullptr;
            bufferMemory.MapMemory(pData_dst, size);
            memoryUsage = size;
            return pData_dst;
        }
        void UnmapMemory() {
            bufferMemory.UnmapMemory(memoryUsage);
            memoryUsage = 0;
        }
        void BufferData(const void* pData_src, VkDeviceSize size) {
            Expand(size);
            bufferMemory.BufferData(pData_src, size);
        }
        [[nodiscard]]
        VkImage AliasedImage2d(VkFormat format, VkExtent2D extent) {
            if (!(FormatProperties(format).linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
                return VK_NULL_HANDLE;
            VkDeviceSize imageDataSize = VkDeviceSize(FormatInfo(format).sizePerPixel) * extent.width * extent.height;
            if (imageDataSize > AllocationSize())
                return VK_NULL_HANDLE;
            VkImageFormatProperties imageFormatProperties = {};
            vkGetPhysicalDeviceImageFormatProperties(graphicsBase::Base().PhysicalDevice(),
                                                     format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, &imageFormatProperties);
            if (extent.width > imageFormatProperties.maxExtent.width ||
                extent.height > imageFormatProperties.maxExtent.height ||
                imageDataSize > imageFormatProperties.maxResourceSize)
                return VK_NULL_HANDLE;
            VkImageCreateInfo imageCreateInfo = {
                    .imageType = VK_IMAGE_TYPE_2D,
                    .format = format,
                    .extent = { extent.width, extent.height, 1 },
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .tiling = VK_IMAGE_TILING_LINEAR,
                    .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
            };
            aliasedImage.~image();
            aliasedImage.Create(imageCreateInfo);
            VkImageSubresource subResource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
            VkSubresourceLayout subresourceLayout = {};
            vkGetImageSubresourceLayout(graphicsBase::Base().Device(), aliasedImage, &subResource, &subresourceLayout);
            if (subresourceLayout.size != imageDataSize)
                return VK_NULL_HANDLE;//No padding bytes
            aliasedImage.BindMemory(bufferMemory.Memory());
            return aliasedImage;
        }
        //Static Function
        static VkBuffer Buffer_MainThread() {
            return stagingBuffer_mainThread.Get();
        }
        static void Expand_MainThread(VkDeviceSize size) {
            stagingBuffer_mainThread.Get().Expand(size);
        }
        static void Release_MainThread() {
            stagingBuffer_mainThread.Get().Release();
        }
        static void* MapMemory_MainThread(VkDeviceSize size) {
            return stagingBuffer_mainThread.Get().MapMemory(size);
        }
        static void UnmapMemory_MainThread() {
            stagingBuffer_mainThread.Get().UnmapMemory();
        }
        static void BufferData_MainThread(const void* pData_src, VkDeviceSize size) {
            stagingBuffer_mainThread.Get().BufferData(pData_src, size);
        }
        static void RetrieveData_MainThread(void* pData_src, VkDeviceSize size) {
            stagingBuffer_mainThread.Get().RetrieveData(pData_src, size);
        }
        [[nodiscard]]
        static VkImage AliasedImage2d_MainThread(VkFormat format, VkExtent2D extent) {
            return stagingBuffer_mainThread.Get().AliasedImage2d(format, extent);
        }
    };

    class deviceLocalBuffer {
    protected:
        myVulkan::bufferMemory bufferMemory;
    public:
        deviceLocalBuffer() = default;
        deviceLocalBuffer(VkDeviceSize size, VkBufferUsageFlags desiredUsages_Without_transfer_dst) {
            Create(size, desiredUsages_Without_transfer_dst);
        }
        //Getter
        operator VkBuffer() const { return bufferMemory.Buffer(); }
        const VkBuffer* Address() const { return bufferMemory.AddressOfBuffer(); }
        VkDeviceSize AllocationSize() const { return bufferMemory.AllocationSize(); }
        //Const Function
        void TransferData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0) const {
            if (bufferMemory.MemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                bufferMemory.BufferData(pData_src, size, offset);
                return;
            }
            stagingBuffer::BufferData_MainThread(pData_src, size);
            auto& commandBuffer = graphicsBase::Plus().CommandBuffer_Transfer();
            commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            VkBufferCopy region = { 0, offset, size };
            vkCmdCopyBuffer(commandBuffer, stagingBuffer::Buffer_MainThread(), bufferMemory.Buffer(), 1, &region);
            commandBuffer.End();
            graphicsBase::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
        }
        void TransferData(const void* pData_src, uint32_t elementCount, VkDeviceSize elementSize, VkDeviceSize stride_src, VkDeviceSize stride_dst, VkDeviceSize offset = 0) const {
            if (bufferMemory.MemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                void* pData_dst = nullptr;
                bufferMemory.MapMemory(pData_dst, stride_dst * elementCount, offset);
                for (size_t i = 0; i < elementCount; i++)
                    memcpy(stride_dst * i + static_cast<uint8_t*>(pData_dst), stride_src * i + static_cast<const uint8_t*>(pData_src), size_t(elementSize));
                bufferMemory.UnmapMemory(elementCount * stride_dst, offset);
                return;
            }
            stagingBuffer::BufferData_MainThread(pData_src, stride_src * elementCount);
            auto& commandBuffer = graphicsBase::Plus().CommandBuffer_Transfer();
            commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            std::unique_ptr<VkBufferCopy[]> regions = std::make_unique<VkBufferCopy[]>(elementCount);
            for (size_t i = 0; i < elementCount; i++)
                regions[i] = { stride_src * i, stride_dst * i + offset, elementSize };
            vkCmdCopyBuffer(commandBuffer, stagingBuffer::Buffer_MainThread(), bufferMemory.Buffer(), elementCount, regions.get());
            commandBuffer.End();
            graphicsBase::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
        }
        void TransferData(const auto& data_src) const {
            TransferData(&data_src, sizeof data_src);
        }
        void CmdUpdateBuffer(VkCommandBuffer commandBuffer, const void* pData_src, VkDeviceSize size_Limited_to_65536, VkDeviceSize offset = 0) const {
            vkCmdUpdateBuffer(commandBuffer, bufferMemory.Buffer(), offset, size_Limited_to_65536, pData_src);
        }
        void CmdUpdateBuffer(VkCommandBuffer commandBuffer, const auto& data_src) const {
            vkCmdUpdateBuffer(commandBuffer, bufferMemory.Buffer(), 0, sizeof data_src, &data_src);
        }
        //Non-const Function
        void Create(VkDeviceSize size, VkBufferUsageFlags desiredUsages_Without_transfer_dst) {
            VkBufferCreateInfo bufferCreateInfo = {
                    .size = size,
                    .usage = desiredUsages_Without_transfer_dst | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            };

            bufferMemory.CreateBuffer(bufferCreateInfo) != 0 ||
            bufferMemory.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            bufferMemory.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ||
            bufferMemory.BindMemory();
        }
        void Recreate(VkDeviceSize size, VkBufferUsageFlags desiredUsages_Without_transfer_dst) {
            graphicsBase::Base().WaitIdle();
            bufferMemory.~bufferMemory();
            Create(size, desiredUsages_Without_transfer_dst);
        }
    };

    class vertexBuffer :public deviceLocalBuffer {
    public:
        vertexBuffer() = default;
        vertexBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) :deviceLocalBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsages) {}
        //Non-const Function
        void Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Create(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsages);
        }
        void Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsages);
        }
    };

    class indexBuffer :public deviceLocalBuffer {
    public:
        indexBuffer() = default;
        indexBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) :deviceLocalBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | otherUsages) {}
        //Non-const Function
        void Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Create(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | otherUsages);
        }
        void Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | otherUsages);
        }
    };

}