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

#include <cstdlib>
#include "window.hpp"

#include "rendering/Device.hpp"
#include "rendering/SwapChain.hpp"

#include "rendering/Pipeline.hpp"

#include "rendering/Mesh.hpp"

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

void submit(const Device& device, VkCommandBuffer& commandBuffer, VkSemaphore imageAvailable, VkSemaphore& renderFinished, VkFence& inFlight) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinished;

    vkResetFences(device.device(), 1, &inFlight);

    ASSERT_VK_SUCCESS(vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlight), "Failed to submit draw command buffer!");
}

void present(const Device& device, const SwapChain& swapChain, uint32_t& imageIndex, size_t& currentFrame, std::vector<VkSemaphore>& imageAvailable, VkSemaphore& renderFinished, VkFence& inFlight, std::vector<VkFence>& imagesInFlight)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinished;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.swapChain();
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;// Optional
    VkResult result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        //resize();
    } else if (result != VK_SUCCESS) {
        spdlog::critical("failed to present swap chain image!");
        throw std::runtime_error("Failed presentation.");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    vkQueueWaitIdle(device.presentQueue());

resizeLoop:
    vkWaitForFences(device.device(), 1, &inFlight, VK_TRUE, UINT64_MAX);
    result = vkAcquireNextImageKHR(device.device(), swapChain.swapChain(), UINT64_MAX, imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        //resize();
        goto resizeLoop;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        spdlog::critical("failed to acquire swap chain image!");
        throw std::runtime_error("Failed presentation.");
    }

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device.device(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    imagesInFlight[imageIndex] = inFlight;
}

int main()
{
    Window window(1280, 720, "Vk App");
    VkPhysicalDeviceFeatures targetFeatures{};
    Device device{ window.context(), window.surface(), targetFeatures };
    SwapChain sc{ window.surface(), device };

    BasicRasterPipeline basicRasterPipeline{ device, sc };

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    uint32_t imageIndex;
    size_t currentFrame = 0;

    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    Model triangle;

    triangle.addFace(
        triangle.addVertex(glm::vec3(-1, 1, 0)),  
        triangle.addVertex(glm::vec3(1, 1, 0)),
        triangle.addVertex(glm::vec3(0, -1, 0)));

    triangle.finalize();

    Mesh vkMesh{device, triangle.getVertices(), triangle.getIndices()};

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(sc.images().size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        static constexpr const char* semaphore_error = "Failed to create semaphores!";
        ASSERT_VK_SUCCESS(vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]), semaphore_error);
        ASSERT_VK_SUCCESS(vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]), semaphore_error);
        ASSERT_VK_SUCCESS(vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]), semaphore_error);
    }

    ASSERT_VK_SUCCESS(vkAcquireNextImageKHR(device.device(), sc.swapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex), "Failed to acquire swap chain image!");

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    framebuffers.resize(sc.imageViews().size());

    for (size_t i = 0; i < sc.imageViews().size(); i++) {
        std::array<VkImageView, 2> attachments = {
            sc.imageViews()[i],
            sc.depthImage()->imageView()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = basicRasterPipeline.m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = sc.extent().width;
        framebufferInfo.height = sc.extent().height;
        framebufferInfo.layers = 1;

        ASSERT_VK_SUCCESS (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffers[i]), "failed to create framebuffer!");
    }

    commandBuffers.resize(framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = device.graphicsPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    ASSERT_VK_SUCCESS(vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()), "failed to allocate command buffers!");

    for(size_t cbfIndex = 0; cbfIndex < commandBuffers.size(); cbfIndex++) {
        auto& commandBuffer = commandBuffers[cbfIndex];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;// Optional
        beginInfo.pInheritanceInfo = nullptr;// Optional

        ASSERT_VK_SUCCESS (vkBeginCommandBuffer(commandBuffer, &beginInfo), "failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = basicRasterPipeline.m_renderPass;
        renderPassInfo.framebuffer = framebuffers[cbfIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = sc.extent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {.float32 = {0.f, 0.f, 0.f, 1.f}};
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, basicRasterPipeline.m_pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(sc.extent().width);
        viewport.height = static_cast<float>(sc.extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = sc.extent();

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkMesh.record_draw_command(commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        ASSERT_VK_SUCCESS (vkEndCommandBuffer(commandBuffer), "failed to record command buffer!");
    }

    while (!window.shouldClose()) {
        glfwPollEvents();

        submit(device, commandBuffers[imageIndex], imageAvailableSemaphores[currentFrame], renderFinishedSemaphores[currentFrame], inFlightFences[currentFrame]);
        present(device, sc, imageIndex, currentFrame, imageAvailableSemaphores, renderFinishedSemaphores[currentFrame], inFlightFences[currentFrame], imagesInFlight);
    }

    if(commandBuffers.size() != 0)
        vkFreeCommandBuffers(device.device(), device.graphicsPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.device(), inFlightFences[i], nullptr);
    }

    return EXIT_SUCCESS;
}