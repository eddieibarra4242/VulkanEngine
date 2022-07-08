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
#include <algorithm>

#include "Device.hpp"

class Buffer
{
  public:
    [[nodiscard]] static constexpr VkDeviceSize align(size_t size, VkDeviceSize alignment)
    {
        return (static_cast<VkDeviceSize>(size) + alignment - 1) & ~(alignment - 1);
    }

    Buffer(const Device& device, VkDeviceSize instanceSize, VkDeviceSize instanceCount, VkDeviceSize alignment, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Buffer();

    [[nodiscard]] constexpr const void* data() const { return m_pMap; }
    [[nodiscard]] constexpr void* data() { return m_pMap; }

    [[nodiscard]] constexpr const VkBuffer& buffer() const { return m_buffer; }

    [[nodiscard]] constexpr VkDeviceSize size() const { return m_size; }
    [[nodiscard]] constexpr VkDeviceSize instanceSize() const { return m_alignedInstanceSize; }
    [[nodiscard]] constexpr VkDeviceSize instanceCount() const { return m_instanceCount; }

    [[nodiscard]] inline VkDescriptorBufferInfo& descInfo(uint32_t index)
    {
        m_bufInfo[index].buffer = m_buffer;
        m_bufInfo[index].offset = index * m_alignedInstanceSize;
        m_bufInfo[index].range = m_alignedInstanceSize;

        return m_bufInfo[index];
    }

    void write(const void* data, size_t size, VkDeviceSize index, size_t offsetInIndex = 0);

    // FIXME: We can copy here.
    DELETE_COPY(Buffer);

    void operator=(Buffer&& other);
    inline Buffer(Buffer&& other) : m_buffer{ VK_NULL_HANDLE }, m_buf_mem{ VK_NULL_HANDLE }, m_pMap{ nullptr }, m_parent_dev(other.m_parent_dev) { this->operator=(std::move(other)); }

  private:
    VkBuffer m_buffer;
    VkDeviceMemory m_buf_mem;

    VkDeviceSize m_size;
    VkDeviceSize m_alignedInstanceSize;
    VkDeviceSize m_instanceCount;

    VkBufferUsageFlags m_usage;
    VkMemoryPropertyFlags m_properties;

    std::vector<VkDescriptorBufferInfo> m_bufInfo;

    void* m_pMap = nullptr;

    const Device& m_parent_dev;

    void create_internal(const Buffer* other, const Device& device, VkDeviceSize instanceSize, VkDeviceSize instanceCount, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroy_internal();
};