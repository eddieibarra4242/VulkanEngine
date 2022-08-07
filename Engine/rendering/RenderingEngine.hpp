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

#pragma once

#include <vector>

#include "Pipeline.hpp"
#include "Mesh.hpp"
#include "Descriptors.hpp"

struct CameraInfo
{
    glm::mat4 m_viewProjection;
};

class RenderingEngine
{
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  public:
    RenderingEngine(const VkSurfaceKHR& surface, Device& device);
    ~RenderingEngine();

    void render();
    void present();

  private:
    const Device& m_device;
    std::unique_ptr<SwapChain> m_swapChain;

    std::unique_ptr<BasicRasterPipeline> m_basicRasterPipeline;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    uint32_t m_imageIndex;
    size_t m_currentFrame = 0;

    bool m_commandBuffersInvalidated = false;
    uint32_t m_startingCBUpdateIndex = static_cast<uint32_t>(-1);
    std::vector<VkFramebuffer> m_framebuffers;
    std::vector<VkCommandBuffer> m_commandBuffers;

    CameraInfo m_mainCamera;
    Buffer m_globalUBO;
    DescriptorSetLayout m_globalLayout;
    DescriptorPool m_globalPool;
    std::vector<VkDescriptorSet> m_globalSets{ MAX_FRAMES_IN_FLIGHT };

    void createFramebuffers();
    void destroyFramebuffers();

    void resize();
    void recordCommandBuffer(uint32_t cbfIndex);

    constexpr void invalidateCommandBuffers()
    {
        if (m_commandBuffersInvalidated) {
            m_startingCBUpdateIndex = static_cast<uint32_t>(-1);
        }

        m_commandBuffersInvalidated = true;
    }
};
