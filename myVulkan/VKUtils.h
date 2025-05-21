//
// Created by Jhong on 2025/5/1.
//

#pragma once
#include "VKBase.hpp"


namespace myVulkan{
    class fence {
        VkFence handle = VK_NULL_HANDLE;
    public:
        //fence() = default;
        fence(VkFenceCreateInfo& createInfo) {
            Create(createInfo);
        }
        //默认构造器创建未置位的栅栏
        fence(VkFenceCreateFlags flags = 0) {
            Create(flags);
        }
        fence(fence&& other) noexcept { MoveHandle; }
        ~fence() { DestroyHandleBy(vkDestroyFence); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        result_t Wait() const {
            VkResult result = vkWaitForFences(graphicsBase::Base().Device(), 1, &handle, false, UINT64_MAX);
            if (result)
                outStream << std::format("[ fence ] ERROR\nFailed to wait for the fence!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Reset() const {
            VkResult result = vkResetFences(graphicsBase::Base().Device(), 1, &handle);
            if (result)
                outStream << std::format("[ fence ] ERROR\nFailed to reset the fence!\nError code: {}\n", int32_t(result));
            return result;
        }
        //因为“等待后立刻重置”的情形经常出现，定义此函数
        result_t WaitAndReset() const {
            VkResult result = Wait();
            result || (result = Reset());
            return result;
        }
        result_t Status() const {
            VkResult result = vkGetFenceStatus(graphicsBase::Base().Device(), handle);
            if (result < 0) //vkGetFenceStatus(...)成功时有两种结果，所以不能仅仅判断result是否非0
                outStream << std::format("[ fence ] ERROR\nFailed to get the status of the fence!\nError code: {}\n", int32_t(result));
            return result;
        }
        //Non-const Function
        result_t Create(VkFenceCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            VkResult result = vkCreateFence(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ fence ] ERROR\nFailed to create a fence!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Create(VkFenceCreateFlags flags = 0) {
            VkFenceCreateInfo createInfo = {
                    .flags = flags
            };
            return Create(createInfo);
        }
    };

    class semaphore {
        VkSemaphore handle = VK_NULL_HANDLE;
    public:
        //semaphore() = default;
        semaphore(VkSemaphoreCreateInfo& createInfo) {
            Create(createInfo);
        }
        //默认构造器创建未置位的信号量
        semaphore(/*VkSemaphoreCreateFlags flags*/) {
            Create();
        }
        semaphore(semaphore&& other) noexcept { MoveHandle; }
        ~semaphore() { DestroyHandleBy(vkDestroySemaphore); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkSemaphoreCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VkResult result = vkCreateSemaphore(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ semaphore ] ERROR\nFailed to create a semaphore!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Create(/*VkSemaphoreCreateFlags flags*/) {
            VkSemaphoreCreateInfo createInfo = {};
            return Create(createInfo);
        }
    };

    class commandBuffer {
        friend class commandPool;//封装命令池的commandPool类负责分配和释放命令缓冲区，需要让其能访问私有成员handle
        VkCommandBuffer handle = VK_NULL_HANDLE;
    public:
        commandBuffer() = default;
        commandBuffer(commandBuffer&& other) noexcept { MoveHandle }
        //因释放命令缓冲区的函数被我定义在封装命令池的commandPool类中，没析构器
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        //这里没给inheritanceInfo设定默认参数，因为C++标准中规定对空指针解引用是未定义行为（尽管运行期不必发生，且至少MSVC编译器允许这种代码），而我又一定要传引用而非指针，因而形成了两个Begin(...)
        result_t Begin(VkCommandBufferUsageFlags usageFlags, VkCommandBufferInheritanceInfo& inheritanceInfo) const {
            inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            VkCommandBufferBeginInfo beginInfo = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = usageFlags,
                    .pInheritanceInfo = &inheritanceInfo
            };
            VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
            if (result)
                outStream << std::format("[ commandBuffer ] ERROR\nFailed to begin a command buffer!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Begin(VkCommandBufferUsageFlags usageFlags = 0) const {
            VkCommandBufferBeginInfo beginInfo = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = usageFlags,
            };
            VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
            if (result)
                outStream << std::format("[ commandBuffer ] ERROR\nFailed to begin a command buffer!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t End() const {
            VkResult result = vkEndCommandBuffer(handle);
            if (result)
                outStream << std::format("[ commandBuffer ] ERROR\nFailed to end a command buffer!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    class commandPool {
        VkCommandPool handle = VK_NULL_HANDLE;
    public:
        commandPool() = default;
        commandPool(VkCommandPoolCreateInfo& createInfo) {
            Create(createInfo);
        }
        commandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0) {
            Create(queueFamilyIndex, flags);
        }
        commandPool(commandPool&& other) noexcept { MoveHandle; }
        ~commandPool() { DestroyHandleBy(vkDestroyCommandPool); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        result_t AllocateBuffers(arrayRef<VkCommandBuffer> buffers, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
            VkCommandBufferAllocateInfo allocateInfo = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = handle,
                    .level = level,
                    .commandBufferCount = uint32_t(buffers.Count())
            };
            VkResult result = vkAllocateCommandBuffers(graphicsBase::Base().Device(), &allocateInfo, buffers.Pointer());
            if (result)
                outStream << std::format("[ commandPool ] ERROR\nFailed to allocate command buffers!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t AllocateBuffers(arrayRef<commandBuffer> buffers, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const {
            return AllocateBuffers(
                    { &buffers[0].handle, buffers.Count() },
                    level);
        }
        void FreeBuffers(arrayRef<VkCommandBuffer> buffers) const {
            vkFreeCommandBuffers(graphicsBase::Base().Device(), handle, buffers.Count(), buffers.Pointer());
            memset(buffers.Pointer(), 0, buffers.Count() * sizeof(VkCommandBuffer));
        }
        void FreeBuffers(arrayRef<commandBuffer> buffers) const {
            FreeBuffers({ &buffers[0].handle, buffers.Count() });
        }
        //Non-const Function
        result_t Create(VkCommandPoolCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            VkResult result = vkCreateCommandPool(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ commandPool ] ERROR\nFailed to create a command pool!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0) {
            VkCommandPoolCreateInfo createInfo = {
                    .flags = flags,
                    .queueFamilyIndex = queueFamilyIndex
            };
            return Create(createInfo);
        }
    };

    //渲染通道
    class renderPass {
        VkRenderPass handle = VK_NULL_HANDLE;
    public:
        renderPass() = default;
        renderPass(VkRenderPassCreateInfo& createInfo) {
            Create(createInfo);
        }
        renderPass(renderPass&& other) noexcept { MoveHandle; }
        ~renderPass() { DestroyHandleBy(vkDestroyRenderPass); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        void CmdBegin(VkCommandBuffer commandBuffer, VkRenderPassBeginInfo& beginInfo, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = handle;
            vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
        }
        void CmdBegin(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkRect2D renderArea, arrayRef<const VkClearValue> clearValues = {}, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
            VkRenderPassBeginInfo beginInfo = {
                    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass = handle,
                    .framebuffer = framebuffer,
                    .renderArea = renderArea,
                    .clearValueCount = uint32_t(clearValues.Count()),
                    .pClearValues = clearValues.Pointer()
            };
            vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
        }
        void CmdNext(VkCommandBuffer commandBuffer, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
            vkCmdNextSubpass(commandBuffer, subpassContents);
        }
        void CmdEnd(VkCommandBuffer commandBuffer) const {
            vkCmdEndRenderPass(commandBuffer);
        }
        //Non-const Function
        result_t Create(VkRenderPassCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            VkResult result = vkCreateRenderPass(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ renderPass ] ERROR\nFailed to create a render pass!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    class framebuffer {
        VkFramebuffer handle = VK_NULL_HANDLE;
    public:
        framebuffer() = default;
        framebuffer(VkFramebufferCreateInfo& createInfo) {
            Create(createInfo);
        }
        framebuffer(framebuffer&& other) noexcept { MoveHandle; }
        ~framebuffer() { DestroyHandleBy(vkDestroyFramebuffer); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkFramebufferCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            VkResult result = vkCreateFramebuffer(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ framebuffer ] ERROR\nFailed to create a framebuffer!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    // 渲染管线布局
    class pipelineLayout {
        VkPipelineLayout handle = VK_NULL_HANDLE;
    public:
        pipelineLayout() = default;
        pipelineLayout(VkPipelineLayoutCreateInfo& createInfo) {
            Create(createInfo);
        }
        pipelineLayout(pipelineLayout&& other) noexcept { MoveHandle; }
        ~pipelineLayout() { DestroyHandleBy(vkDestroyPipelineLayout); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkPipelineLayoutCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            VkResult result = vkCreatePipelineLayout(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ pipelineLayout ] ERROR\nFailed to create a pipeline layout!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    // 渲染管线封装
    class pipeline {
        VkPipeline handle = VK_NULL_HANDLE;
    public:
        pipeline() = default;
        pipeline(VkGraphicsPipelineCreateInfo& createInfo) {
            Create(createInfo);
        }
        pipeline(VkComputePipelineCreateInfo& createInfo) {
            Create(createInfo);
        }
        pipeline(pipeline&& other) noexcept { MoveHandle; }
        ~pipeline() { DestroyHandleBy(vkDestroyPipeline); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkGraphicsPipelineCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            VkResult result = vkCreateGraphicsPipelines(graphicsBase::Base().Device(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ pipeline ] ERROR\nFailed to create a graphics pipeline!\nError code: {}\n", int32_t(result));
                    return result;
        }
        result_t Create(VkComputePipelineCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            VkResult result = vkCreateComputePipelines(graphicsBase::Base().Device(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ pipeline ] ERROR\nFailed to create a compute pipeline!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    //封装着色器模组
    class shaderModule {
        VkShaderModule handle = VK_NULL_HANDLE;
    public:
        shaderModule() = default;
        shaderModule(VkShaderModuleCreateInfo& createInfo) {
            Create(createInfo);
        }
        shaderModule(const char* filepath /*VkShaderModuleCreateFlags flags*/) {
            Create(filepath);
        }
        shaderModule(size_t codeSize, const uint32_t* pCode /*VkShaderModuleCreateFlags flags*/) {
            Create(codeSize, pCode);
        }
        shaderModule(shaderModule&& other) noexcept { MoveHandle; }
        ~shaderModule() { DestroyHandleBy(vkDestroyShaderModule); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        VkPipelineShaderStageCreateInfo StageCreateInfo(VkShaderStageFlagBits stage, const char* entry = "main") const {
            return {
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,//sType
                    nullptr,                                            //pNext
                    0,                                                  //flags
                    stage,                                              //stage
                    handle,                                             //module
                    entry,                                              //pName
                    nullptr                                             //pSpecializationInfo
            };
        }
        //Non-const Function
        result_t Create(VkShaderModuleCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            VkResult result = vkCreateShaderModule(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ shader ] ERROR\nFailed to create a shader module!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Create(const char* filepath /*VkShaderModuleCreateFlags flags*/) {
            std::ifstream file(filepath, std::ios::ate | std::ios::binary);
            if (!file) {
                outStream << std::format("[ shader ] ERROR\nFailed to open the file: {}\n", filepath);
                return VK_RESULT_MAX_ENUM;//没有合适的错误代码，别用VK_ERROR_UNKNOWN
            }
            size_t fileSize = size_t(file.tellg());
            std::vector<uint32_t> binaries(fileSize / 4);
            file.seekg(0);
            file.read(reinterpret_cast<char*>(binaries.data()), fileSize);
            file.close();
            return Create(fileSize, binaries.data());
        }
        result_t Create(size_t codeSize, const uint32_t* pCode /*VkShaderModuleCreateFlags flags*/) {
            VkShaderModuleCreateInfo createInfo = {
                    .codeSize = codeSize,
                    .pCode = pCode
            };
            return Create(createInfo);
        }
    };


    //封装设备内存分配
    class deviceMemory {
        VkDeviceMemory handle = VK_NULL_HANDLE;
        VkDeviceSize allocationSize = 0; //实际分配的内存大小
        VkMemoryPropertyFlags memoryProperties = 0; //内存属性
        //--------------------
        //该函数用于在映射内存区时，调整非host coherent的内存区域的范围
        VkDeviceSize AdjustNonCoherentMemoryRange(VkDeviceSize& size, VkDeviceSize& offset) const {
            const VkDeviceSize& nonCoherentAtomSize = graphicsBase::Base().PhysicalDeviceProperties().limits.nonCoherentAtomSize;
            VkDeviceSize _offset = offset;
            offset = offset / nonCoherentAtomSize * nonCoherentAtomSize;
            size = std::min((size + _offset + nonCoherentAtomSize - 1) / nonCoherentAtomSize * nonCoherentAtomSize, allocationSize) - offset;
            return _offset - offset;
        }
    protected:
        //用于bufferMemory或imageMemory，定义于此以节省8个字节
        class {
            friend class bufferMemory;
            friend class imageMemory;
            bool value = false;
            operator bool() const { return value; }
            auto& operator=(bool value) { this->value = value; return *this; }
        } areBound;
    public:
        deviceMemory() = default;
        deviceMemory(VkMemoryAllocateInfo& allocateInfo) {
            Allocate(allocateInfo);
        }
        deviceMemory(deviceMemory&& other) noexcept {
            MoveHandle;
            allocationSize = other.allocationSize;
            memoryProperties = other.memoryProperties;
            other.allocationSize = 0;
            other.memoryProperties = 0;
        }
        ~deviceMemory() { DestroyHandleBy(vkFreeMemory); allocationSize = 0; memoryProperties = 0; }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        VkDeviceSize AllocationSize() const { return allocationSize; }
        VkMemoryPropertyFlags MemoryProperties() const { return memoryProperties; }
        //Const Function
        //映射host visible的内存区
        result_t MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset = 0) const {
            VkDeviceSize inverseDeltaOffset;
            if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                inverseDeltaOffset = AdjustNonCoherentMemoryRange(size, offset);
            if (VkResult result = vkMapMemory(graphicsBase::Base().Device(), handle, offset, size, 0, &pData)) {
                outStream << std::format("[ deviceMemory ] ERROR\nFailed to map the memory!\nError code: {}\n", int32_t(result));
                return result;
            }
            if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                pData = static_cast<uint8_t*>(pData) + inverseDeltaOffset;
                VkMappedMemoryRange mappedMemoryRange = {
                        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                        .memory = handle,
                        .offset = offset,
                        .size = size
                };
                if (VkResult result = vkInvalidateMappedMemoryRanges(graphicsBase::Base().Device(), 1, &mappedMemoryRange)) {
                    outStream << std::format("[ deviceMemory ] ERROR\nFailed to flush the memory!\nError code: {}\n", int32_t(result));
                    return result;
                }
            }
            return VK_SUCCESS;
        }
        //取消映射host visible的内存区
        result_t UnmapMemory(VkDeviceSize size, VkDeviceSize offset = 0) const {
            if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                AdjustNonCoherentMemoryRange(size, offset);
                VkMappedMemoryRange mappedMemoryRange = {
                        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                        .memory = handle,
                        .offset = offset,
                        .size = size
                };
                if (VkResult result = vkFlushMappedMemoryRanges(graphicsBase::Base().Device(), 1, &mappedMemoryRange)) {
                    outStream << std::format("[ deviceMemory ] ERROR\nFailed to flush the memory!\nError code: {}\n", int32_t(result));
                    return result;
                }
            }
            vkUnmapMemory(graphicsBase::Base().Device(), handle);
            return VK_SUCCESS;
        }
        //BufferData(...)用于方便地更新设备内存区，适用于用memcpy(...)向内存区写入数据后立刻取消映射的情况
        result_t BufferData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0) const {
            void* pData_dst;
            if (VkResult result = MapMemory(pData_dst, size, offset))
                return result;
            memcpy(pData_dst, pData_src, size_t(size));
            return UnmapMemory(size, offset);
        }
        result_t BufferData(const auto& data_src) const {
            return BufferData(&data_src, sizeof data_src);
        }
        //RetrieveData(...)用于方便地从设备内存区取回数据，适用于用memcpy(...)从内存区取得数据后立刻取消映射的情况
        result_t RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset = 0) const {
            void* pData_src;
            if (VkResult result = MapMemory(pData_src, size, offset))
                return result;
            memcpy(pData_dst, pData_src, size_t(size));
            return UnmapMemory(size, offset);
        }
        //Non-const Function
        result_t Allocate(VkMemoryAllocateInfo& allocateInfo) {
            if (allocateInfo.memoryTypeIndex >= graphicsBase::Base().PhysicalDeviceMemoryProperties().memoryTypeCount) {
                outStream << std::format("[ deviceMemory ] ERROR\nInvalid memory type index!\n");
                return VK_RESULT_MAX_ENUM; //没有合适的错误代码，别用VK_ERROR_UNKNOWN
            }
            allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            if (VkResult result = vkAllocateMemory(graphicsBase::Base().Device(), &allocateInfo, nullptr, &handle)) {
                outStream << std::format("[ deviceMemory ] ERROR\nFailed to allocate memory!\nError code: {}\n", int32_t(result));
                return result;
            }
            //记录实际分配的内存大小
            allocationSize = allocateInfo.allocationSize;
            //取得内存属性
            memoryProperties = graphicsBase::Base().PhysicalDeviceMemoryProperties().memoryTypes[allocateInfo.memoryTypeIndex].propertyFlags;
            return VK_SUCCESS;
        }
    };

    //封装缓冲区
    class buffer {
        VkBuffer handle = VK_NULL_HANDLE;
    public:
        buffer() = default;
        buffer(VkBufferCreateInfo& createInfo) {
            Create(createInfo);
        }
        buffer(buffer&& other) noexcept { MoveHandle; }
        ~buffer() { DestroyHandleBy(vkDestroyBuffer); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        VkMemoryAllocateInfo MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const {
            VkMemoryAllocateInfo memoryAllocateInfo = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
            };
            VkMemoryRequirements memoryRequirements;
            vkGetBufferMemoryRequirements(graphicsBase::Base().Device(), handle, &memoryRequirements);
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            memoryAllocateInfo.memoryTypeIndex = UINT32_MAX;
            auto& physicalDeviceMemoryProperties = graphicsBase::Base().PhysicalDeviceMemoryProperties();
            for (size_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
                if (memoryRequirements.memoryTypeBits & 1 << i &&
                    (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & desiredMemoryProperties) == desiredMemoryProperties) {
                    memoryAllocateInfo.memoryTypeIndex = i;
                    break;
                }
            //不在此检查是否成功取得内存类型索引，因为会把memoryAllocateInfo返回出去，交由外部检查
            //if (memoryAllocateInfo.memoryTypeIndex == UINT32_MAX)
            //    outStream << std::format("[ buffer ] ERROR\nFailed to find any memory type satisfies all desired memory properties!\n");
            return memoryAllocateInfo;
        }
        result_t BindMemory(VkDeviceMemory deviceMemory, VkDeviceSize memoryOffset = 0) const {
            VkResult result = vkBindBufferMemory(graphicsBase::Base().Device(), handle, deviceMemory, memoryOffset);
            if (result)
                outStream << std::format("[ buffer ] ERROR\nFailed to attach the memory!\nError code: {}\n", int32_t(result));
            return result;
        }
        //Non-const Function
        result_t Create(VkBufferCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            VkResult result = vkCreateBuffer(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ buffer ] ERROR\nFailed to create a buffer!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    //封装缓冲区和设备内存
    class bufferMemory :buffer, deviceMemory {
    public:
        bufferMemory() = default;
        bufferMemory(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties) {
            Create(createInfo, desiredMemoryProperties);
        }
        bufferMemory(bufferMemory&& other) noexcept :
                buffer(std::move(other)), deviceMemory(std::move(other)) {
            areBound = other.areBound;
            other.areBound = false;
        }
        ~bufferMemory() { areBound = false; }
        //Getter
        //不定义到VkBuffer和VkDeviceMemory的转换函数，因为32位下这俩类型都是uint64_t的别名，会造成冲突（虽然，谁他妈还用32位PC！）
        VkBuffer Buffer() const { return static_cast<const buffer&>(*this); }
        const VkBuffer* AddressOfBuffer() const { return buffer::Address(); }
        VkDeviceMemory Memory() const { return static_cast<const deviceMemory&>(*this); }
        const VkDeviceMemory* AddressOfMemory() const { return deviceMemory::Address(); }
        //若areBond为true，则成功分配了设备内存、创建了缓冲区，且成功绑定在一起
        bool AreBound() const { return areBound; }
        using deviceMemory::AllocationSize;
        using deviceMemory::MemoryProperties;
        //Const Function
        using deviceMemory::MapMemory;
        using deviceMemory::UnmapMemory;
        using deviceMemory::BufferData;
        using deviceMemory::RetrieveData;
        //Non-const Function
        //以下三个函数仅用于Create(...)可能执行失败的情况
        result_t CreateBuffer(VkBufferCreateInfo& createInfo) {
            return buffer::Create(createInfo);
        }
        result_t AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties) {
            VkMemoryAllocateInfo allocateInfo = MemoryAllocateInfo(desiredMemoryProperties);
            if (allocateInfo.memoryTypeIndex >= graphicsBase::Base().PhysicalDeviceMemoryProperties().memoryTypeCount)
                return VK_RESULT_MAX_ENUM; //没有合适的错误代码，别用VK_ERROR_UNKNOWN
            return Allocate(allocateInfo);
        }
        result_t BindMemory() {
            if (VkResult result = buffer::BindMemory(Memory()))
                return result;
            areBound = true;
            return VK_SUCCESS;
        }
        //分配设备内存、创建缓冲、绑定
        result_t Create(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties) {
            VkResult result;
            (result = CreateBuffer(createInfo)) != 0 || //用||短路执行
            (result = AllocateMemory(desiredMemoryProperties)) ||
            (result = BindMemory());
            return result;
        }
    };

    //缓冲区视图（VkBufferView）定义了将纹素缓冲区作为1D图像使用的方式
    class bufferView {
        VkBufferView handle = VK_NULL_HANDLE;
    public:
        bufferView() = default;
        bufferView(VkBufferViewCreateInfo& createInfo) {
            Create(createInfo);
        }
        bufferView(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0, VkDeviceSize range = 0 /*VkBufferViewCreateFlags flags*/) {
            Create(buffer, format, offset, range);
        }
        bufferView(bufferView&& other) noexcept { MoveHandle; }
        ~bufferView() { DestroyHandleBy(vkDestroyBufferView); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkBufferViewCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            VkResult result = vkCreateBufferView(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ bufferView ] ERROR\nFailed to create a buffer view!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Create(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0, VkDeviceSize range = 0 /*VkBufferViewCreateFlags flags*/) {
            VkBufferViewCreateInfo createInfo = {
                    .buffer = buffer,
                    .format = format,
                    .offset = offset,
                    .range = range
            };
            return Create(createInfo);
        }
    };

    //图像（VkImage）引用设备内存，指代图像数据
    class image {
        VkImage handle = VK_NULL_HANDLE;
    public:
        image() = default;
        image(VkImageCreateInfo& createInfo) {
            Create(createInfo);
        }
        image(image&& other) noexcept { MoveHandle; }
        ~image() { DestroyHandleBy(vkDestroyImage); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        VkMemoryAllocateInfo MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const {
            VkMemoryAllocateInfo memoryAllocateInfo = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
            };
            VkMemoryRequirements memoryRequirements;
            vkGetImageMemoryRequirements(graphicsBase::Base().Device(), handle, &memoryRequirements);
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            auto GetMemoryTypeIndex = [](uint32_t memoryTypeBits, VkMemoryPropertyFlags desiredMemoryProperties) {
                auto& physicalDeviceMemoryProperties = graphicsBase::Base().PhysicalDeviceMemoryProperties();
                for (size_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
                    if (memoryTypeBits & 1 << i &&
                        (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & desiredMemoryProperties) == desiredMemoryProperties)
                        return static_cast<uint32_t>(i);
                return UINT32_MAX;
            };
            memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, desiredMemoryProperties);
            if (memoryAllocateInfo.memoryTypeIndex == UINT32_MAX &&
                desiredMemoryProperties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, desiredMemoryProperties & ~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
            //不在此检查是否成功取得内存类型索引，因为会把memoryAllocateInfo返回出去，交由外部检查
            //if (memoryAllocateInfo.memoryTypeIndex == -1)
            //    outStream << std::format("[ image ] ERROR\nFailed to find any memory type satisfies all desired memory properties!\n");
            return memoryAllocateInfo;
        }
        result_t BindMemory(VkDeviceMemory deviceMemory, VkDeviceSize memoryOffset = 0) const {
            VkResult result = vkBindImageMemory(graphicsBase::Base().Device(), handle, deviceMemory, memoryOffset);
            if (result)
                outStream << std::format("[ image ] ERROR\nFailed to attach the memory!\nError code: {}\n", int32_t(result));
            return result;
        }
        //Non-const Function
        result_t Create(VkImageCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            VkResult result = vkCreateImage(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ image ] ERROR\nFailed to create an image!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    //同设备内存一起封装为imageMemory类
    class imageMemory :image, deviceMemory {
    public:
        imageMemory() = default;
        imageMemory(VkImageCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties) {
            Create(createInfo, desiredMemoryProperties);
        }
        imageMemory(imageMemory&& other) noexcept :
                image(std::move(other)), deviceMemory(std::move(other)) {
            areBound = other.areBound;
            other.areBound = false;
        }
        ~imageMemory() { areBound = false; }
        //Getter
        VkImage Image() const { return static_cast<const image&>(*this); }
        const VkImage* AddressOfImage() const { return image::Address(); }
        VkDeviceMemory Memory() const { return static_cast<const deviceMemory&>(*this); }
        const VkDeviceMemory* AddressOfMemory() const { return deviceMemory::Address(); }
        bool AreBound() const { return areBound; }
        using deviceMemory::AllocationSize;
        using deviceMemory::MemoryProperties;
        //Non-const Function
        //以下三个函数仅用于Create(...)可能执行失败的情况
        result_t CreateImage(VkImageCreateInfo& createInfo) {
            return image::Create(createInfo);
        }
        result_t AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties) {
            VkMemoryAllocateInfo allocateInfo = MemoryAllocateInfo(desiredMemoryProperties);
            if (allocateInfo.memoryTypeIndex >= graphicsBase::Base().PhysicalDeviceMemoryProperties().memoryTypeCount)
                return VK_RESULT_MAX_ENUM; //没有合适的错误代码，别用VK_ERROR_UNKNOWN
            return Allocate(allocateInfo);
        }
        result_t BindMemory() {
            if (VkResult result = image::BindMemory(Memory()))
                return result;
            areBound = true;
            return VK_SUCCESS;
        }
        //分配设备内存、创建图像、绑定
        result_t Create(VkImageCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties) {
            VkResult result;
            (result = CreateImage(createInfo)) != 0 || //用||短路执行
            (result = AllocateMemory(desiredMemoryProperties)) ||
            (result = BindMemory());
            return result;
        }
    };

    //图像视图（VkImageView）定义了图像的使用方式
    class imageView {
        VkImageView handle = VK_NULL_HANDLE;
    public:
        imageView() = default;
        imageView(VkImageViewCreateInfo& createInfo) {
            Create(createInfo);
        }
        imageView(VkImage image, VkImageViewType viewType, VkFormat format, const VkImageSubresourceRange& subresourceRange, VkImageViewCreateFlags flags = 0) {
            Create(image, viewType, format, subresourceRange, flags);
        }
        imageView(imageView&& other) noexcept { MoveHandle; }
        ~imageView() {
            DestroyHandleBy(vkDestroyImageView);
        }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkImageViewCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            VkResult result = vkCreateImageView(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ imageView ] ERROR\nFailed to create an image view!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Create(VkImage image, VkImageViewType viewType, VkFormat format, const VkImageSubresourceRange& subresourceRange, VkImageViewCreateFlags flags = 0) {
            VkImageViewCreateInfo createInfo = {
                    .flags = flags,
                    .image = image,
                    .viewType = viewType,
                    .format = format,
                    .subresourceRange = subresourceRange
            };
            return Create(createInfo);
        }
    };

    // 描述符布局（VkDescriptorSetLayout）包含了管线如何使用描述符的信息
    class descriptorSetLayout {
        VkDescriptorSetLayout handle = VK_NULL_HANDLE;
    public:
        descriptorSetLayout() = default;
        descriptorSetLayout(VkDescriptorSetLayoutCreateInfo& createInfo) {
            Create(createInfo);
        }
        descriptorSetLayout(descriptorSetLayout&& other) noexcept { MoveHandle; }
        ~descriptorSetLayout() { DestroyHandleBy(vkDestroyDescriptorSetLayout); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkDescriptorSetLayoutCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            VkResult result = vkCreateDescriptorSetLayout(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ descriptorSetLayout ] ERROR\nFailed to create a descriptor set layout!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    // 着色器中被成套使用的一系列描述符的集合
    class descriptorSet {
        friend class descriptorPool;
        VkDescriptorSet handle = VK_NULL_HANDLE;
    public:
        descriptorSet() = default;
        descriptorSet(descriptorSet&& other) noexcept { MoveHandle; }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        void Write(arrayRef<const VkDescriptorImageInfo> descriptorInfos, VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const {
            VkWriteDescriptorSet writeDescriptorSet = {
                    .dstSet = handle,
                    .dstBinding = dstBinding,
                    .dstArrayElement = dstArrayElement,
                    .descriptorCount = uint32_t(descriptorInfos.Count()),
                    .descriptorType = descriptorType,
                    .pImageInfo = descriptorInfos.Pointer()
            };
            Update(writeDescriptorSet);
        }
        void Write(arrayRef<const VkDescriptorBufferInfo> descriptorInfos, VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const {
            VkWriteDescriptorSet writeDescriptorSet = {
                    .dstSet = handle,
                    .dstBinding = dstBinding,
                    .dstArrayElement = dstArrayElement,
                    .descriptorCount = uint32_t(descriptorInfos.Count()),
                    .descriptorType = descriptorType,
                    .pBufferInfo = descriptorInfos.Pointer()
            };
            Update(writeDescriptorSet);
        }
        void Write(arrayRef<const VkBufferView> descriptorInfos, VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const {
            VkWriteDescriptorSet writeDescriptorSet = {
                    .dstSet = handle,
                    .dstBinding = dstBinding,
                    .dstArrayElement = dstArrayElement,
                    .descriptorCount = uint32_t(descriptorInfos.Count()),
                    .descriptorType = descriptorType,
                    .pTexelBufferView = descriptorInfos.Pointer()
            };
            Update(writeDescriptorSet);
        }
        void Write(arrayRef<const bufferView> descriptorInfos, VkDescriptorType descriptorType, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0) const {
            Write({ descriptorInfos[0].Address(), descriptorInfos.Count() }, descriptorType, dstBinding, dstArrayElement);
        }
        //Static Function
        static void Update(arrayRef<VkWriteDescriptorSet> writes, arrayRef<VkCopyDescriptorSet> copies = {}) {
            for (auto& i : writes)
                i.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            for (auto& i : copies)
                i.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
            vkUpdateDescriptorSets(
                    graphicsBase::Base().Device(), writes.Count(), writes.Pointer(), copies.Count(), copies.Pointer());
        }
    };

    // 描述符池，用于创建描述符集
    class descriptorPool {
        VkDescriptorPool handle = VK_NULL_HANDLE;
    public:
        descriptorPool() = default;
        descriptorPool(VkDescriptorPoolCreateInfo& createInfo) {
            Create(createInfo);
        }
        descriptorPool(uint32_t maxSetCount, arrayRef<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags = 0) {
            Create(maxSetCount, poolSizes, flags);
        }
        descriptorPool(descriptorPool&& other) noexcept { MoveHandle; }
        ~descriptorPool() { DestroyHandleBy(vkDestroyDescriptorPool); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Const Function
        result_t AllocateSets(arrayRef<VkDescriptorSet> sets, arrayRef<const VkDescriptorSetLayout> setLayouts) const {
            if (sets.Count() != setLayouts.Count())
                if (sets.Count() < setLayouts.Count()) {
                    outStream << std::format("[ descriptorPool ] ERROR\nFor each descriptor set, must provide a corresponding layout!\n");
                    return VK_RESULT_MAX_ENUM;//没有合适的错误代码，别用VK_ERROR_UNKNOWN
                }
                else
                    outStream << std::format("[ descriptorPool ] WARNING\nProvided layouts are more than sets!\n");
            VkDescriptorSetAllocateInfo allocateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .descriptorPool = handle,
                    .descriptorSetCount = uint32_t(sets.Count()),
                    .pSetLayouts = setLayouts.Pointer()
            };
            VkResult result = vkAllocateDescriptorSets(graphicsBase::Base().Device(), &allocateInfo, sets.Pointer());
            if (result)
                outStream << std::format("[ descriptorPool ] ERROR\nFailed to allocate descriptor sets!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t AllocateSets(arrayRef<VkDescriptorSet> sets, arrayRef<const descriptorSetLayout> setLayouts) const {
            return AllocateSets(
                    sets,
                    { setLayouts[0].Address(), setLayouts.Count() });
        }
        result_t AllocateSets(arrayRef<descriptorSet> sets, arrayRef<const VkDescriptorSetLayout> setLayouts) const {
            return AllocateSets(
                    { &sets[0].handle, sets.Count() },
                    setLayouts);
        }
        result_t AllocateSets(arrayRef<descriptorSet> sets, arrayRef<const descriptorSetLayout> setLayouts) const {
            return AllocateSets(
                    { &sets[0].handle, sets.Count() },
                    { setLayouts[0].Address(), setLayouts.Count() });
        }
        result_t FreeSets(arrayRef<VkDescriptorSet> sets) const {
            VkResult result = vkFreeDescriptorSets(graphicsBase::Base().Device(), handle, sets.Count(), sets.Pointer());
            memset(sets.Pointer(), 0, sets.Count() * sizeof(VkDescriptorSet));
            return result;//Though vkFreeDescriptorSets(...) can only return VK_SUCCESS
        }
        result_t FreeSets(arrayRef<descriptorSet> sets) const {
            return FreeSets({ &sets[0].handle, sets.Count() });
        }
        //Non-const Function
        result_t Create(VkDescriptorPoolCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            VkResult result = vkCreateDescriptorPool(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ descriptorPool ] ERROR\nFailed to create a descriptor pool!\nError code: {}\n", int32_t(result));
            return result;
        }
        result_t Create(uint32_t maxSetCount, arrayRef<const VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags = 0) {
            VkDescriptorPoolCreateInfo createInfo = {
                    .flags = flags,
                    .maxSets = maxSetCount,
                    .poolSizeCount = uint32_t(poolSizes.Count()),
                    .pPoolSizes = poolSizes.Pointer()
            };
            return Create(createInfo);
        }
    };

    // 采样器
    class sampler {
        VkSampler handle = VK_NULL_HANDLE;
    public:
        sampler() = default;
        sampler(VkSamplerCreateInfo& createInfo) {
            Create(createInfo);
        }
        sampler(sampler&& other) noexcept { MoveHandle; }
        ~sampler() { DestroyHandleBy(vkDestroySampler); }
        //Getter
        DefineHandleTypeOperator;
        DefineAddressFunction;
        //Non-const Function
        result_t Create(VkSamplerCreateInfo& createInfo) {
            createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            VkResult result = vkCreateSampler(graphicsBase::Base().Device(), &createInfo, nullptr, &handle);
            if (result)
                outStream << std::format("[ sampler ] ERROR\nFailed to create a sampler!\nError code: {}\n", int32_t(result));
            return result;
        }
    };

    class attachment {
    protected:
        myVulkan::imageView imageView;
        myVulkan::imageMemory imageMemory;
        //--------------------
        attachment() = default;
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
    };

    class colorAttachment :public attachment {
    public:
        colorAttachment() = default;
        colorAttachment(VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0) {
            Create(format, extent, layerCount, sampleCount, otherUsages);
        }
        //Non-const Function
        void Create(VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0) {
                VkImageCreateInfo imageCreateInfo = {
                    .imageType = VK_IMAGE_TYPE_2D,
                    .format = format,
                    .extent = { extent.width, extent.height, 1 },
                    .mipLevels = 1,
                    .arrayLayers = layerCount,
                    .samples = sampleCount,
                    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | otherUsages
                };
                imageMemory.Create(
                    imageCreateInfo,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | bool(otherUsages & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) * VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
                imageView.Create(
                    imageMemory.Image(),
                    layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
                    format,
                    { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount });
        }
        //Static Function
        //该函数用于检查某一格式的图像可否被用作颜色附件
        static bool FormatAvailability(VkFormat format, bool supportBlending = true);
    };
    class depthStencilAttachment :public attachment {
    public:
        depthStencilAttachment() = default;
        depthStencilAttachment(VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0, bool stencilOnly = false) {
            Create(format, extent, layerCount, sampleCount, otherUsages, stencilOnly);
        }
        //Non-const Function
        void Create(VkFormat format, VkExtent2D extent, uint32_t layerCount = 1,
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0, bool stencilOnly = false) {
            VkImageCreateInfo imageCreateInfo = {
                .imageType = VK_IMAGE_TYPE_2D,
                .format = format,
                .extent = { extent.width, extent.height, 1 },
                .mipLevels = 1,
                .arrayLayers = layerCount,
                .samples = sampleCount,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | otherUsages
            };
            imageMemory.Create(
                imageCreateInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | bool(otherUsages & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) * VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
            //确定aspcet mask-------------------------
            VkImageAspectFlags aspectMask = (!stencilOnly) * VK_IMAGE_ASPECT_DEPTH_BIT;
            if (format > VK_FORMAT_S8_UINT)
                aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            else if (format == VK_FORMAT_S8_UINT)
                aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            //----------------------------------------
            imageView.Create(
                imageMemory.Image(),
                layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
                format,
                { aspectMask, 0, 1, 0, layerCount });
        }
        //Static Function
        //该函数用于检查某一格式的图像可否被用作深度模板附件
        static bool FormatAvailability(VkFormat format);
    };


}
