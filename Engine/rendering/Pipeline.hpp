#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "SwapChain.hpp"

struct Pipeline
{
    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};

class BasicRasterPipeline : public Pipeline
{
  public:
    BasicRasterPipeline(Device& device, const SwapChain& swapChain, const std::vector<VkDescriptorSetLayout>& layouts = std::vector<VkDescriptorSetLayout>());
    virtual ~BasicRasterPipeline();

    DELETE_COPY(BasicRasterPipeline);
    // FIXME: Implement move functions

  private:
    const VkDevice m_parentDev;
};