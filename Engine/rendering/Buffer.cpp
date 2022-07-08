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

#include "Buffer.hpp"

#include "VulkanUtils.hpp"

#include <memory.h>

void createBuffer(const Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ASSERT_VK_SUCCESS(vkCreateBuffer(device.device(), &bufferInfo, nullptr, &buffer), "failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.device(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, properties);

    ASSERT_VK_SUCCESS(vkAllocateMemory(device.device(), &allocInfo, nullptr, &bufferMemory), "failed to allocate buffer memory!");

    vkBindBufferMemory(device.device(), buffer, bufferMemory, 0);
}

Buffer::Buffer(const Device& device, VkDeviceSize instanceSize, VkDeviceSize instanceCount, VkDeviceSize alignment, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : m_instanceCount{ instanceCount }, m_usage{ usage }, m_properties{ properties }, m_parent_dev{ device }
{
    m_alignedInstanceSize = align(instanceSize, alignment);
    m_size = m_alignedInstanceSize * m_instanceCount;

    m_bufInfo.resize(m_instanceCount);

    createBuffer(device, m_size, m_usage, m_properties, m_buffer, m_buf_mem);

    if ((m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        vkMapMemory(device.device(), m_buf_mem, 0, m_size, 0, &m_pMap);
    }
}

Buffer::~Buffer()
{
    destroy_internal();
}

void Buffer::destroy_internal()
{
    if (m_pMap != nullptr) {
        vkUnmapMemory(m_parent_dev.device(), m_buf_mem);
    }

    if (m_buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_parent_dev.device(), m_buffer, nullptr);
        vkFreeMemory(m_parent_dev.device(), m_buf_mem, nullptr);
    }
}

void Buffer::operator=(Buffer&& other)
{
    destroy_internal();

    this->m_buffer = other.m_buffer;
    this->m_buf_mem = other.m_buf_mem;

    this->m_size = other.m_size;
    this->m_alignedInstanceSize = other.m_alignedInstanceSize;
    this->m_instanceCount = other.m_instanceCount;

    this->m_usage = other.m_usage;
    this->m_properties = other.m_properties;

    this->m_pMap = other.m_pMap;

    other.m_buffer = VK_NULL_HANDLE;
    other.m_buf_mem = VK_NULL_HANDLE;

    other.m_pMap = nullptr;
}

void Buffer::write(const void* data, size_t size, VkDeviceSize index, size_t offsetInIndex)
{
    if (size == 0) {
        return;
    }

    if (m_pMap != nullptr) {
        VkDeviceSize indexOffset = index * m_alignedInstanceSize;
        char* dest = reinterpret_cast<char*>(m_pMap) + indexOffset + offsetInIndex;
        memcpy(dest, data, size);

        if ((m_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            // FIXME: use nonCoherentAtomSize when calculating offset and size here.

            // flush the whole index
            VkMappedMemoryRange mappedRange = {};
            mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = m_buf_mem;
            mappedRange.offset = indexOffset;
            mappedRange.size = m_alignedInstanceSize;
            vkFlushMappedMemoryRanges(m_parent_dev.device(), 1, &mappedRange);
        }

        return;
    }

    VkBuffer staging;
    VkDeviceMemory smem;
    createBuffer(m_parent_dev, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, smem);

    void* staging_dest;
    vkMapMemory(m_parent_dev.device(), smem, 0, size, 0, &staging_dest);
    memcpy(staging_dest, data, size);
    vkUnmapMemory(m_parent_dev.device(), smem);

    size = std::min(size, m_size);

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_parent_dev.device(), m_parent_dev.graphicsPool());

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = index * m_alignedInstanceSize;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, staging, m_buffer, 1, &copyRegion);

    endSingleTimeCommands(m_parent_dev.device(), m_parent_dev.graphicsQueue(), m_parent_dev.graphicsPool(), commandBuffer);

    vkDestroyBuffer(m_parent_dev.device(), staging, nullptr);
    vkFreeMemory(m_parent_dev.device(), smem, nullptr);
}