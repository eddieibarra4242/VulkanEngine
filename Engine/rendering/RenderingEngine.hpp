#pragma once

#include <vector>

#include "Pipeline.hpp"
#include "Mesh.hpp"

class RenderingEngine
{
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

public:
    RenderingEngine(const VkSurfaceKHR& surface, Device& device);
    ~RenderingEngine();

    void render();
    void present();

    //Temporary
    inline void addMesh(const std::shared_ptr<Mesh> mesh) {
        m_meshes.push_back(mesh);
    }

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

    std::vector<std::shared_ptr<Mesh>> m_meshes;

    void createFramebuffers();
    void destroyFramebuffers();

    void resize();
    void recordCommandBuffer(uint32_t cbfIndex);

    constexpr void invalidateCommandBuffers() 
    {
        if(m_commandBuffersInvalidated) {
            m_startingCBUpdateIndex = static_cast<uint32_t>(-1); 
        }
        
        m_commandBuffersInvalidated = true; 
    }
};