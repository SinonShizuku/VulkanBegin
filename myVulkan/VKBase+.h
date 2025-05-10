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

    // 统一缓冲区
    class uniformBuffer :public deviceLocalBuffer {
    public:
        uniformBuffer() = default;
        uniformBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) :deviceLocalBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages) {}
        //Non-const Function
        void Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages);
        }
        void Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages);
        }
        //Static Function
        static VkDeviceSize CalculateAlignedSize(VkDeviceSize dataSize) {
            const VkDeviceSize& alignment = graphicsBase::Base().PhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
            return dataSize + alignment - 1 & ~(alignment - 1);
        }
    };

    class storageBuffer :public deviceLocalBuffer {
    public:
        storageBuffer() = default;
        storageBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) :deviceLocalBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | otherUsages) {}
        //Non-const Function
        void Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Create(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | otherUsages);
        }
        void Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
            deviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | otherUsages);
        }
        //Static Function
        static VkDeviceSize CalculateAlignedSize(VkDeviceSize dataSize) {
            const VkDeviceSize& alignment = graphicsBase::Base().PhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
            return dataSize + alignment - 1 & ~(alignment - 1);
        }
    };

    struct imageOperation {
        struct imageMemoryBarrierParameterPack {
            const bool isNeeded = false;                            //是否需要屏障，默认为false
            const VkPipelineStageFlags stage = 0;                   //srcStages或dstStages
            const VkAccessFlags access = 0;                         //srcAccessMask或dstAccessMask
            const VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED; //oldLayout或newLayout
            //默认构造器，isNeeded保留为false
            constexpr imageMemoryBarrierParameterPack() = default;
            //若指定参数，三个参数必须被全部显示指定，isNeeded被赋值为true
            constexpr imageMemoryBarrierParameterPack(VkPipelineStageFlags stage, VkAccessFlags access, VkImageLayout layout) :
                    isNeeded(true), stage(stage), access(access), layout(layout) {}
        };
        static void CmdCopyBufferToImage(
                VkCommandBuffer commandBuffer,   //命令缓冲区的handle
                VkBuffer buffer,                 //作为数据来源的缓冲区
                VkImage image,                   //接收数据的目标图像
                const VkBufferImageCopy& region, //指定将缓冲区的哪些部分拷贝到图像的哪些部分
                imageMemoryBarrierParameterPack imb_from, //后述
                imageMemoryBarrierParameterPack imb_to) {

            VkImageMemoryBarrier imageMemoryBarrier = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    nullptr,
                    imb_from.access,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    imb_from.layout,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_QUEUE_FAMILY_IGNORED, //无队列族所有权转移
                    VK_QUEUE_FAMILY_IGNORED,
                    image,
                    {
                            region.imageSubresource.aspectMask,
                            region.imageSubresource.mipLevel,
                            1,
                            region.imageSubresource.baseArrayLayer,
                            region.imageSubresource.layerCount }
            };
            if (imb_from.isNeeded)
                vkCmdPipelineBarrier(commandBuffer, imb_from.stage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                     0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            if (imb_to.isNeeded) {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.dstAccessMask = imb_to.access;
                imageMemoryBarrier.newLayout = imb_to.layout;
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, imb_to.stage, 0,
                                     0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
        }
        static void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage image_src, VkImage image_dst, const VkImageBlit& region,
                                 imageMemoryBarrierParameterPack imb_dst_from, imageMemoryBarrierParameterPack imb_dst_to, VkFilter filter = VK_FILTER_LINEAR) {
            VkImageMemoryBarrier imageMemoryBarrier = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    nullptr,
                    imb_dst_from.access,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    imb_dst_from.layout,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                    image_dst,
                    {
                            region.dstSubresource.aspectMask,
                            region.dstSubresource.mipLevel,
                            1,
                            region.dstSubresource.baseArrayLayer,
                            region.dstSubresource.layerCount }
            };
            if (imb_dst_from.isNeeded)
                vkCmdPipelineBarrier(commandBuffer, imb_dst_from.stage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                     0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            vkCmdBlitImage(commandBuffer,
                           image_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           image_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region, filter);
            if (imb_dst_to.isNeeded) {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.dstAccessMask = imb_dst_to.access;
                imageMemoryBarrier.newLayout = imb_dst_to.layout;
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, imb_dst_to.stage, 0,
                                     0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
        }

        //出问题了看https://easyvulkan.github.io/Ch5-2%202D%E8%B4%B4%E5%9B%BE%E5%8F%8A%E7%94%9F%E6%88%90Mipmap.html
        static void CmdGenerateMipmap2d(VkCommandBuffer commandBuffer, VkImage image, VkExtent2D imageExtent, uint32_t mipLevelCount, uint32_t layerCount,
                                        imageMemoryBarrierParameterPack imb_to, VkFilter minFilter = VK_FILTER_LINEAR) {
            auto MipmapExtent = [](VkExtent2D imageExtent, uint32_t mipLevel) {
                VkOffset3D extent = { int32_t(imageExtent.width >> mipLevel), int32_t(imageExtent.height >> mipLevel), 1 };
                extent.x += !extent.x;
                extent.y += !extent.y;
                return extent;
            };
            for (uint32_t i = 1; i < mipLevelCount; i++) {
                VkImageBlit region = {
                        { VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, layerCount },//srcSubresource
                        { {}, MipmapExtent(imageExtent, i - 1) },           //srcOffsets
                        { VK_IMAGE_ASPECT_COLOR_BIT, i, 0, layerCount },    //dstSubresource
                        { {}, MipmapExtent(imageExtent, i) }                //dstOffsets
                };
                CmdBlitImage(commandBuffer, image, image, region,
                             { VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                             { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL }, minFilter);
            }
            if (imb_to.isNeeded) {
                VkImageMemoryBarrier imageMemoryBarrier = {
                        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        nullptr,
                        0,
                        imb_to.access,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        imb_to.layout,
                        VK_QUEUE_FAMILY_IGNORED,
                        VK_QUEUE_FAMILY_IGNORED,
                        image,
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, layerCount }
                };
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, imb_to.stage, 0,
                                     0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }
        }
    };

    class texture {
    protected:
        myVulkan::imageView imageView;
        myVulkan::imageMemory imageMemory;
        //--------------------
        texture() = default;
        //该函数用于方便地创建imageMemory
        void CreateImageMemory(VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageCreateFlags flags = 0) {
            VkImageCreateInfo imageCreateInfo = {
                    .flags = flags,
                    .imageType = imageType,
                    .format = format,
                    .extent = extent,
                    .mipLevels = mipLevelCount,
                    .arrayLayers = arrayLayerCount,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            };
            imageMemory.Create(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
        //该函数用于方便地创建imageView
        void CreateImageView(VkImageViewType viewType, VkFormat format, uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageViewCreateFlags flags = 0) {
            imageView.Create(imageMemory.Image(), viewType, format, { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, arrayLayerCount }, flags);
        }
        //根据硬盘或内存地址读取资源
        static std::unique_ptr<uint8_t[]> LoadFile_Internal(const auto* address, size_t fileSize, VkExtent2D& extent, formatInfo requiredFormatInfo) {
#ifndef NDEBUG
            if (!(requiredFormatInfo.rawDataType == formatInfo::floatingPoint && requiredFormatInfo.sizePerComponent == 4) &&
                !(requiredFormatInfo.rawDataType == formatInfo::integer && Between_Closed<int32_t>(1, requiredFormatInfo.sizePerComponent, 2)))
                outStream << std::format("[ texture ] ERROR\nRequired format is not available for source image data!\n"),
                        abort();
#endif
            int& width = reinterpret_cast<int&>(extent.width);
            int& height = reinterpret_cast<int&>(extent.height);
            int channelCount;
            void* pImageData = nullptr;
            if constexpr (std::same_as<decltype(address), const char*>) {
                if (requiredFormatInfo.rawDataType == formatInfo::integer)
                    if (requiredFormatInfo.sizePerComponent == 1)
                        pImageData = stbi_load(address, &width, &height, &channelCount, requiredFormatInfo.componentCount);
                    else
                        pImageData = stbi_load_16(address, &width, &height, &channelCount, requiredFormatInfo.componentCount);
                else
                    pImageData = stbi_loadf(address, &width, &height, &channelCount, requiredFormatInfo.componentCount);
                if (!pImageData)
                    outStream << std::format("[ texture ] ERROR\nFailed to load the file: {}\n", address);
            }
            if constexpr (std::same_as<decltype(address), const uint8_t*>) {
                if (fileSize > INT32_MAX) {
                    outStream << std::format("[ texture ] ERROR\nFailed to load image data from the given address! Data size must be less than 2G!\n");
                    return {};
                }
                if (requiredFormatInfo.rawDataType == formatInfo::integer)
                    if (requiredFormatInfo.sizePerComponent == 1)
                        pImageData = stbi_load_from_memory(address, fileSize, &width, &height, &channelCount, requiredFormatInfo.componentCount);
                    else
                        pImageData = stbi_load_16_from_memory(address, fileSize, &width, &height, &channelCount, requiredFormatInfo.componentCount);
                else
                    pImageData = stbi_loadf_from_memory(address, fileSize, &width, &height, &channelCount, requiredFormatInfo.componentCount);
                if (!pImageData)
                    outStream << std::format("[ texture ] ERROR\nFailed to load image data from the given address!\n");
            }
            return std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(pImageData));
        }
    public:
        //Getter
        VkImageView ImageView() const { return imageView; }
        VkImage Image() const { return imageMemory.Image(); }
        const VkImageView* AddressOfImageView() const { return imageView.Address(); }
        const VkImage* AddressOfImage() const { return imageMemory.AddressOfImage(); }
        //Const Function
        //该函数返回写入描述符时需要的信息
        VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler) const {
            return { sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        }
        [[nodiscard]]
        static std::unique_ptr<uint8_t[]> LoadFile(const char* filepath, VkExtent2D& extent, formatInfo requiredFormatInfo) {
            return LoadFile_Internal(filepath, 0, extent, requiredFormatInfo);
        }
        [[nodiscard]]
        static std::unique_ptr<uint8_t[]> LoadFile(const uint8_t* fileBinaries, size_t fileSize, VkExtent2D& extent, formatInfo requiredFormatInfo) {
            return LoadFile_Internal(fileBinaries, fileSize, extent, requiredFormatInfo);
        }
        static uint32_t CalculateMipLevelCount(VkExtent2D extent) {
            return uint32_t(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
        }
        static void CopyBlitAndGenerateMipmap2d(VkBuffer buffer_copyFrom, VkImage image_copyTo, VkImage image_blitTo, VkExtent2D imageExtent,
                                                uint32_t mipLevelCount = 1, uint32_t layerCount = 1, VkFilter minFilter = VK_FILTER_LINEAR) {
            static constexpr imageOperation::imageMemoryBarrierParameterPack imbs[2] = {
                    { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
                    { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL }
            };
            //生成mipmap的条件是mip等级大于1
            bool generateMipmap = mipLevelCount > 1;
            //发生成blit的条件是源图像和目标图像不同
            bool blitMipLevel0 = image_copyTo != image_blitTo;
            //录制命令
            auto& commandBuffer = graphicsBase::Plus().CommandBuffer_Transfer();
            commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

            VkBufferImageCopy region = {
                    .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount },
                    .imageExtent = { imageExtent.width, imageExtent.height, 1 }
            };
            imageOperation::CmdCopyBufferToImage(commandBuffer, buffer_copyFrom, image_copyTo, region,
                                                 { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED }, imbs[generateMipmap || blitMipLevel0]);

            //如有必要，进行blit
            if (blitMipLevel0) {
                VkImageBlit region = {
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount },
                        { {}, { int32_t(imageExtent.width), int32_t(imageExtent.height), 1 } },
                        { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount },
                        { {}, { int32_t(imageExtent.width), int32_t(imageExtent.height), 1 } }
                };
                imageOperation::CmdBlitImage(commandBuffer, image_copyTo, image_blitTo, region,
                                             { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED }, imbs[generateMipmap], minFilter);
            }

            //如有必要，生成mipmap
            if (generateMipmap)
                imageOperation::CmdGenerateMipmap2d(commandBuffer, image_blitTo, imageExtent, mipLevelCount, layerCount,
                                                    { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, minFilter);


            commandBuffer.End();
            //执行命令
            graphicsBase::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
        }

        static void BlitAndGenerateMipmap2d(VkImage image_preinitialized, VkImage image_final, VkExtent2D imageExtent,
                                            uint32_t mipLevelCount = 1, uint32_t layerCount = 1, VkFilter minFilter = VK_FILTER_LINEAR) {
            //生成mipmap的条件是mip等级大于1
            bool generateMipmap = mipLevelCount > 1;
            //发生成blit的条件是源图像和目标图像不同
            bool blitMipLevel0 = image_preinitialized != image_final;
            if (generateMipmap || blitMipLevel0) {
                auto& commandBuffer = graphicsBase::Plus().CommandBuffer_Transfer();
                commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

                //如有必要，从image_preinitialized将原图blit到image_final
                if (blitMipLevel0) {
                    VkImageMemoryBarrier imageMemoryBarrier = {
                            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                            nullptr,
                            0,
                            VK_ACCESS_TRANSFER_READ_BIT,
                            VK_IMAGE_LAYOUT_PREINITIALIZED,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            VK_QUEUE_FAMILY_IGNORED,
                            VK_QUEUE_FAMILY_IGNORED,
                            image_preinitialized,
                            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount }
                    };
                    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                         0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
                    VkImageBlit region = {
                            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount },
                            { {}, { int32_t(imageExtent.width), int32_t(imageExtent.height), 1 } },
                            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount },
                            { {}, { int32_t(imageExtent.width), int32_t(imageExtent.height), 1 } }
                    };
                    if (generateMipmap)
                        imageOperation::CmdBlitImage(commandBuffer, image_preinitialized, image_final, region,
                                                     { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                                                     { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL }, minFilter);
                    else
                        imageOperation::CmdBlitImage(commandBuffer, image_preinitialized, image_final, region,
                                                     { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
                                                     { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, minFilter);
                }

                //如有必要，生成mipmap
                if (generateMipmap)
                    imageOperation::CmdGenerateMipmap2d(commandBuffer, image_final, imageExtent, mipLevelCount, layerCount,
                                                        { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, minFilter);

                commandBuffer.End();
                graphicsBase::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
            }
        }

        static VkSamplerCreateInfo SamplerCreateInfo() {
            return {
                    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .mipLodBias = 0.f,
                    .anisotropyEnable = VK_TRUE,
                    .maxAnisotropy = graphicsBase::Base().PhysicalDeviceProperties().limits.maxSamplerAnisotropy,
                    .compareEnable = VK_FALSE,
                    .compareOp = VK_COMPARE_OP_ALWAYS,
                    .minLod = 0.f,
                    .maxLod = VK_LOD_CLAMP_NONE,
                    .borderColor = {},
                    .unnormalizedCoordinates = VK_FALSE
            };
        }
    };

    class texture2d :public texture {
    protected:
        VkExtent2D extent = {};
        //--------------------
        void Create_Internal(VkFormat format_initial, VkFormat format_final, bool generateMipmap) {
            //计算Mipmap级数
            uint32_t mipLevelCount = generateMipmap ? CalculateMipLevelCount(extent) : 1;
            //创建图像并分配内存
            CreateImageMemory(VK_IMAGE_TYPE_2D, format_final, { extent.width, extent.height, 1 }, mipLevelCount, 1);
            //创建图像视图
            CreateImageView(VK_IMAGE_VIEW_TYPE_2D, format_final, mipLevelCount, 1);
            if (format_initial == format_final)
                //若不需要格式转换，直接从暂存缓冲区拷贝到图像，不发生blit
                CopyBlitAndGenerateMipmap2d(stagingBuffer::Buffer_MainThread(), imageMemory.Image(), imageMemory.Image(), extent, mipLevelCount, 1);
            else
            if (VkImage image_conversion = stagingBuffer::AliasedImage2d_MainThread(format_initial, extent))
                //若需要格式转换，但是能为暂存缓冲区创建混叠图像，则直接blit
                BlitAndGenerateMipmap2d(image_conversion, imageMemory.Image(), extent, mipLevelCount, 1);
            else {
                //否则，创建新的暂存图像用于中转
                VkImageCreateInfo imageCreateInfo = {
                        .imageType = VK_IMAGE_TYPE_2D,
                        .format = format_initial,
                        .extent = { extent.width, extent.height, 1 },
                        .mipLevels = 1,
                        .arrayLayers = 1,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                };
                myVulkan::imageMemory imageMemory_conversion(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                //从暂存缓冲区拷贝到图像，然后再blit
                CopyBlitAndGenerateMipmap2d(stagingBuffer::Buffer_MainThread(), imageMemory_conversion.Image(), imageMemory.Image(), extent, mipLevelCount, 1);
            }

        }
    public:
        texture2d() = default;
        texture2d(const char* filepath, VkFormat format_initial, VkFormat format_final, bool generateMipmap = true) {
            Create(filepath, format_initial, format_final, generateMipmap);
        }
        texture2d(const uint8_t* pImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool generateMipmap = true) {
            Create(pImageData, extent, format_initial, format_final, generateMipmap);
        }
        //Getter
        VkExtent2D Extent() const { return extent; }
        uint32_t Width() const { return extent.width; }
        uint32_t Height() const { return extent.height; }
        //Non-const Function
        //直接从硬盘读取文件
        void Create(const char* filepath, VkFormat format_initial, VkFormat format_final, bool generateMipmap = true) {
            VkExtent2D extent;
            formatInfo formatInfo = FormatInfo(format_initial);//根据指定的format_initial取得格式信息
            std::unique_ptr<uint8_t[]> pImageData = LoadFile(filepath, extent, formatInfo);
            if (pImageData)
                Create(pImageData.get(), extent, format_initial, format_final, generateMipmap);
        }
        //从内存读取文件数据
        void Create(const uint8_t* pImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool generateMipmap = true) {
            this->extent = extent;
            size_t imageDataSize = size_t(FormatInfo(format_initial).sizePerPixel) * extent.width * extent.height;
            stagingBuffer::BufferData_MainThread(pImageData, imageDataSize);//拷贝数据到暂存缓冲区
            Create_Internal(format_initial, format_final, generateMipmap);
        }
    };


}