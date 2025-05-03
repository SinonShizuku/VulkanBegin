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
}
