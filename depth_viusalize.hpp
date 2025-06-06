#pragma once

#include "window/GlfwGeneral.hpp"
#include "myVulkan/VKBase.h"
#include "myVulkan/VKUtils.h"
#include "myVulkan/EasyVulkan.hpp"
#include "myVulkan/VKBase+.h"

#include "scene.h"

// 当前目录
std::filesystem::path rootDir = std::filesystem::path(__FILE__).parent_path();

pipelineLayout pipelineLayout_into3d;
pipeline pipeline_into3d;
const auto& RenderPassAndFramebuffers_Screen() {
    static const auto& rpwf = easyVulkan::CreateRpwf_ScreenWithDS();
    return rpwf;
}
void CreateLayout() {
    VkPushConstantRange pushConstantRange = { VK_SHADER_STAGE_VERTEX_BIT, 0, 64 };
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };
    pipelineLayout_into3d.Create(pipelineLayoutCreateInfo);
}

void CreatePipeline() {
	static shaderModule vert((rootDir / "shader3d/Into3D.vert.spv").c_str());
	static shaderModule frag((rootDir / "shader3d/Into3D.frag.spv").c_str());
	static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_into3d[2] = {
		vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
		frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
	};
	auto Create = [] {
		graphicsPipelineCreateInfoPack pipelineCiPack;
		pipelineCiPack.createInfo.layout = pipelineLayout_into3d;
		pipelineCiPack.createInfo.renderPass = RenderPassAndFramebuffers_Screen().renderPass;
		pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(vertex3d), VK_VERTEX_INPUT_RATE_VERTEX);
		pipelineCiPack.vertexInputBindings.emplace_back(1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_INSTANCE);
		pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex3d, position));
		pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex3d, color));
		pipelineCiPack.vertexInputAttributes.emplace_back(2, 1, VK_FORMAT_R32G32B32_SFLOAT, 0);
		pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
		pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
		pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_BACK_BIT;
		pipelineCiPack.rasterizationStateCi.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;//Default
		pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_TRUE;
		pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
		pipelineCiPack.depthStencilStateCi.depthCompareOp = VK_COMPARE_OP_LESS;
		pipelineCiPack.colorBlendAttachmentStates.push_back({ .colorWriteMask = 0b1111 });
		pipelineCiPack.UpdateAllArrays();
		pipelineCiPack.createInfo.stageCount = 2;
		pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_into3d;
		pipeline_into3d.Create(pipelineCiPack);
	};
	auto Destroy = [] {
		pipeline_into3d.~pipeline();
	};
	graphicsBase::Base().AddCallback_CreateSwapchain(Create);
	graphicsBase::Base().AddCallback_DestroySwapchain(Destroy);
	Create();
}


int main() {
    if (!InitializeWindow({1280,720}))
        return -1;

    const auto& [renderPass, framebuffers] = RenderPassAndFramebuffers_Screen();
    CreateLayout();
    CreatePipeline();

    fence fence;
    semaphore semaphore_imageIsAvailable;
    semaphore semaphore_renderingIsOver;

    commandBuffer commandBuffer;
    commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.AllocateBuffers(commandBuffer);

    vertex3d vertices[] = {
        //x+
        { {  1,  1, -1 }, { 1, 0, 0, 1 } },
        { {  1, -1, -1 }, { 1, 0, 0, 1 } },
        { {  1,  1,  1 }, { 1, 0, 0, 1 } },
        { {  1, -1,  1 }, { 1, 0, 0, 1 } },
        //x-
        { { -1,  1,  1 }, { 0, 1, 1, 1 } },
        { { -1, -1,  1 }, { 0, 1, 1, 1 } },
        { { -1,  1, -1 }, { 0, 1, 1, 1 } },
        { { -1, -1, -1 }, { 0, 1, 1, 1 } },
        //y+
        { {  1,  1, -1 }, { 0, 1, 0, 1 } },
        { {  1,  1,  1 }, { 0, 1, 0, 1 } },
        { { -1,  1, -1 }, { 0, 1, 0, 1 } },
        { { -1,  1,  1 }, { 0, 1, 0, 1 } },
        //y-
        { {  1, -1, -1 }, { 1, 0, 1, 1 } },
        { { -1, -1, -1 }, { 1, 0, 1, 1 } },
        { {  1, -1,  1 }, { 1, 0, 1, 1 } },
        { { -1, -1,  1 }, { 1, 0, 1, 1 } },
        //z+
        { {  1,  1,  1 }, { 0, 0, 1, 1 } },
        { {  1, -1,  1 }, { 0, 0, 1, 1 } },
        { { -1,  1,  1 }, { 0, 0, 1, 1 } },
        { { -1, -1,  1 }, { 0, 0, 1, 1 } },
        //z-
        { { -1,  1, -1 }, { 1, 1, 0, 1 } },
        { { -1, -1, -1 }, { 1, 1, 0, 1 } },
        { {  1,  1, -1 }, { 1, 1, 0, 1 } },
        { {  1, -1, -1 }, { 1, 1, 0, 1 } }
    };
    vertexBuffer vertexBuffer_perVertex(sizeof vertices);
    vertexBuffer_perVertex.TransferData(vertices);

    glm::vec3 offsets[] = {
        { -4, -4,  6 }, {  4, -4,  6 },
        { -4,  4, 10 }, {  4,  4, 10 },
        { -4, -4, 14 }, {  4, -4, 14 },
        { -4,  4, 18 }, {  4,  4, 18 },
        { -4, -4, 22 }, {  4, -4, 22 },
        { -4,  4, 26 }, {  4,  4, 26 }
    };
    vertexBuffer vertexBuffer_perInstance(sizeof offsets);
    vertexBuffer_perInstance.TransferData(offsets);

    uint16_t indices[36] = { 0, 1, 2, 2, 1, 3 };
    for (size_t i = 1; i < 6; i++)
        for (size_t j = 0; j < 6; j++)
            indices[i * 6 + j] = indices[j] + i * 4;
    indexBuffer indexBuffer(sizeof indices);
    indexBuffer.TransferData(indices);

    glm::mat4 proj = FlipVertical(glm::infinitePerspectiveLH_ZO(glm::radians(60.f), float(windowSize.width) / windowSize.height, 0.1f));

    VkClearValue clearValues[2] = {
        { .color = { 0.f, 0.f, 0.f, 1.f } },
        { .depthStencil = { 1.f, 0 } }
    };

    while (!glfwWindowShouldClose(pWindow)) {
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();

        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
        auto i = graphicsBase::Base().CurrentImageIndex();
        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        renderPass.CmdBegin(commandBuffer, framebuffers[i], { {}, windowSize }, clearValues);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_into3d);
        VkBuffer buffers[2] = { vertexBuffer_perVertex, vertexBuffer_perInstance };
        VkDeviceSize offsets[2] = {};
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdPushConstants(commandBuffer, pipelineLayout_into3d, VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &proj);
        vkCmdDrawIndexed(commandBuffer, 36, 12, 0, 0, 0);
        renderPass.CmdEnd(commandBuffer);

        commandBuffer.End();
        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable, semaphore_renderingIsOver, fence,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
        graphicsBase::Base().PresentImage(semaphore_renderingIsOver);

        glfwPollEvents();
        TitleFPS();

        fence.WaitAndReset();
    }
    TerminateWindow();
    return 0;
}

