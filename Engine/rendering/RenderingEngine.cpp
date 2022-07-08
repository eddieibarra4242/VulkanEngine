/*
   Copyright 2022 Eduardo Ibarra

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "RenderingEngine.hpp"

#include <glm/gtc/matrix_transform.hpp>

RenderingEngine::RenderingEngine(const VkSurfaceKHR& surface, Device& device) : m_device{ device },
                                                                                m_swapChain{ std::make_unique<SwapChain>(surface, device) },
                                                                                m_globalUBO{ device, sizeof(CameraInfo), MAX_FRAMES_IN_FLIGHT, device.physicalDevice().m_properties.limits.minUniformBufferOffsetAlignment, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT },
                                                                                m_globalLayout{ device },
                                                                                m_globalPool{ device }
{
    m_globalLayout.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).seal();
    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(m_globalLayout.layout());
    m_basicRasterPipeline = std::make_unique<BasicRasterPipeline>(device, *m_swapChain.get(), layouts);

    m_globalPool.setMaxSets(MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT).seal();

    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_imagesInFlight.resize(m_swapChain->images().size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        static constexpr const char* semaphore_error = "Failed to create semaphores!";
        ASSERT_VK_SUCCESS(vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]), semaphore_error);
        ASSERT_VK_SUCCESS(vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]), semaphore_error);
        ASSERT_VK_SUCCESS(vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_inFlightFences[i]), semaphore_error);
    }

    ASSERT_VK_SUCCESS(vkAcquireNextImageKHR(m_device.device(), m_swapChain->swapChain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_imageIndex), "Failed to acquire swap chain image!");

    m_imagesInFlight[m_imageIndex] = m_inFlightFences[m_currentFrame];

    createFramebuffers();

    m_commandBuffers.resize(m_framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device.graphicsPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()), "failed to allocate command buffers!");

    invalidateCommandBuffers();

    m_mainCamera.m_viewProjection =
        glm::perspective(glm::pi<float>() / 2.0f, static_cast<float>(m_swapChain->extent().width) / static_cast<float>(m_swapChain->extent().height), 0.1f, 1000.0f) * glm::translate(glm::identity<glm::mat4>(), glm::vec3(0, 0, -3));

    m_mainCamera.m_viewProjection[1][1] *= -1;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_globalUBO.write(&m_mainCamera, sizeof(CameraInfo), i);
        DescriptorSetWriter descWriter{ m_globalLayout, m_globalPool };
        descWriter.writeBuffer(0, &m_globalUBO.descInfo(static_cast<uint32_t>(i)));
        descWriter.createAndWrite(m_globalSets[i]);
    }
}

RenderingEngine::~RenderingEngine()
{
    if (m_commandBuffers.size() != 0) {
        vkFreeCommandBuffers(m_device.device(), m_device.graphicsPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    }

    destroyFramebuffers();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device.device(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device.device(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device.device(), m_inFlightFences[i], nullptr);
    }
}

void RenderingEngine::recordCommandBuffer(uint32_t cbfIndex)
{
    auto& commandBuffer = m_commandBuffers[cbfIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;// Optional
    beginInfo.pInheritanceInfo = nullptr;// Optional

    ASSERT_VK_SUCCESS(vkBeginCommandBuffer(commandBuffer, &beginInfo), "failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_basicRasterPipeline->m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[cbfIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChain->extent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { .float32 = { 0.f, 0.f, 0.f, 1.f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_basicRasterPipeline->m_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain->extent().width);
    viewport.height = static_cast<float>(m_swapChain->extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChain->extent();

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_basicRasterPipeline->m_pipelineLayout, 0, 1, &m_globalSets[m_currentFrame], 0, nullptr);

    for (auto& mesh : m_meshes) {
        mesh->record_draw_command(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);

    ASSERT_VK_SUCCESS(vkEndCommandBuffer(commandBuffer), "failed to record command buffer!");
}

void RenderingEngine::createFramebuffers()
{
    m_framebuffers.resize(m_swapChain->imageViews().size());

    for (size_t i = 0; i < m_swapChain->imageViews().size(); i++) {
        std::array<VkImageView, 2> attachments = {
            m_swapChain->imageViews()[i],
            m_swapChain->depthImage()->imageView()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_basicRasterPipeline->m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapChain->extent().width;
        framebufferInfo.height = m_swapChain->extent().height;
        framebufferInfo.layers = 1;

        ASSERT_VK_SUCCESS(vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &m_framebuffers[i]), "failed to create framebuffer!");
    }
}

void RenderingEngine::destroyFramebuffers()
{
    for (auto framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);
    }
}

void RenderingEngine::resize()
{
    m_swapChain->rebuild();
    destroyFramebuffers();
    createFramebuffers();
    invalidateCommandBuffers();
}

void RenderingEngine::render()
{
    // FIXME: bad style, remove nested if's
    if (m_commandBuffersInvalidated) {
        if (m_startingCBUpdateIndex == static_cast<uint32_t>(-1)) {
            recordCommandBuffer(m_imageIndex);
            m_startingCBUpdateIndex = m_imageIndex;
        } else if (m_startingCBUpdateIndex == m_imageIndex) {
            m_commandBuffersInvalidated = false;
            m_startingCBUpdateIndex = static_cast<uint32_t>(-1);
        } else {
            recordCommandBuffer(m_imageIndex);
        }
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

    vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);

    ASSERT_VK_SUCCESS(vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]), "Failed to submit draw command buffer!");
}

void RenderingEngine::present()
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain->swapChain();
    presentInfo.pImageIndices = &m_imageIndex;
    presentInfo.pResults = nullptr;// Optional
    VkResult result = vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        resize();
    } else if (result != VK_SUCCESS) {
        spdlog::critical("failed to present swap chain image!");
        throw std::runtime_error("Failed presentation.");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    vkQueueWaitIdle(m_device.presentQueue());

resizeLoop:
    vkWaitForFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    result = vkAcquireNextImageKHR(m_device.device(), m_swapChain->swapChain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        resize();
        goto resizeLoop;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        spdlog::critical("failed to acquire swap chain image!");
        throw std::runtime_error("Failed presentation.");
    }

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (m_imagesInFlight[m_imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_device.device(), 1, &m_imagesInFlight[m_imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    m_imagesInFlight[m_imageIndex] = m_inFlightFences[m_currentFrame];
}