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