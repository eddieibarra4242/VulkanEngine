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

#include <vulkan/vulkan.h>
#include <vector>
#include "Device.hpp"

#include "Image.hpp"

class SwapChain
{
  public:
    SwapChain(const VkSurfaceKHR& surface, Device& device);
    ~SwapChain();

    [[nodiscard]] constexpr const VkSwapchainKHR& swapChain() const { return m_swapChain; }
    [[nodiscard]] constexpr const std::vector<VkImage>& images() const { return m_images; }
    [[nodiscard]] constexpr const VkFormat& imageFormat() const { return m_imageFormat; }
    [[nodiscard]] constexpr const VkExtent2D& extent() const { return m_extent; }
    [[nodiscard]] constexpr const std::vector<VkImageView>& imageViews() const { return m_imageViews; }
    [[nodiscard]] constexpr const std::unique_ptr<Image>& depthImage() const { return m_depthImage; }

    void rebuild();

  private:
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_images;
    VkFormat m_imageFormat;
    VkExtent2D m_extent;
    std::vector<VkImageView> m_imageViews;

    std::unique_ptr<Image> m_depthImage;

    Device& m_parent_dev;
    const VkSurfaceKHR& m_surface;

    void create();
    void destroy();
};