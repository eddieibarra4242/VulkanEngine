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

#include "../utils.hpp"
#include "Device.hpp"

class Image
{
  public:
    [[nodiscard]] static constexpr bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }

  public:
    Image(const Device& device, const char* filename, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
    Image(const Device& device, uint32_t width, uint32_t height, void* data, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

    Image(const Device& device, VkExtent2D extent, VkFormat depthFormat);

    ~Image();

    [[nodiscard]] constexpr const VkImage& image() const { return m_image; }
    [[nodiscard]] constexpr const VkImageView& imageView() const { return m_imageView; }

    [[nodiscard]] inline VkDescriptorImageInfo& descInfo()
    {
        m_descInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_descInfo.imageView = m_imageView;
        m_descInfo.sampler = m_sampler;

        return m_descInfo;
    }

    DELETE_COPY(Image);

    void operator=(Image&& other);
    Image(Image&& other);

  private:
    const Device& m_device;

    uint32_t m_width;
    uint32_t m_height;

    // Image info
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;

    // For descriptors
    VkSampler m_sampler = VK_NULL_HANDLE;
    VkDescriptorImageInfo m_descInfo;

    void createInternal(void* data, VkFormat format);
};