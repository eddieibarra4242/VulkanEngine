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

#include "../utils.hpp"

#include <unordered_map>
#include "Device.hpp"

class DescriptorSetLayout
{
  public:
    DescriptorSetLayout(const Device& device) : m_parent_dev{ device } {}
    ~DescriptorSetLayout();

    DescriptorSetLayout& addBinding(uint32_t binding, VkDescriptorType descType, VkShaderStageFlags stageFlags, uint32_t count = 1);
    void seal();

    [[nodiscard]] constexpr const VkDescriptorSetLayout& layout() const { return m_layout; }

    DELETE_COPY(DescriptorSetLayout);

    void operator=(DescriptorSetLayout&& other);
    inline DescriptorSetLayout(DescriptorSetLayout&& other) : m_parent_dev{ other.m_parent_dev }, m_layout{ VK_NULL_HANDLE } { this->operator=(std::move(other)); }

  private:
    const Device& m_parent_dev;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

    bool m_seal = false;

    void destroy_internal();

    friend class DescriptorSetWriter;
};

class DescriptorPool
{
  public:
    DescriptorPool(const Device& device) : m_parent_dev{ device }, m_maxSets{ 1000 } {}
    ~DescriptorPool();

    DescriptorPool& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    DescriptorPool& setPoolFlags(VkDescriptorPoolCreateFlags flags);
    DescriptorPool& setMaxSets(uint32_t count);
    void seal();

    [[nodiscard]] bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
    void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
    void reset();

    [[nodiscard]] constexpr const VkDescriptorPool& pool() const { return m_descriptorPool; }

    DELETE_COPY(DescriptorPool);

    void operator=(DescriptorPool&& other);
    inline DescriptorPool(DescriptorPool&& other) : m_parent_dev{ other.m_parent_dev }, m_descriptorPool{ VK_NULL_HANDLE } { this->operator=(std::move(other)); }

  private:
    const Device& m_parent_dev;

    std::vector<VkDescriptorPoolSize> m_poolSizes;
    uint32_t m_maxSets;
    VkDescriptorPoolCreateFlags m_poolFlags = 0;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    bool m_seal = false;

    void destroy_internal();

    friend class DescriptorSetWriter;
};

class DescriptorSetWriter
{
  public:
    DescriptorSetWriter(DescriptorSetLayout& layout, DescriptorPool& pool) : m_layout{ layout }, m_pool{ pool } {}

    DescriptorSetWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* info);
    DescriptorSetWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* info);

    bool createAndWrite(VkDescriptorSet& set);
    void write(VkDescriptorSet& set);

  private:
    DescriptorSetLayout& m_layout;
    DescriptorPool& m_pool;
    std::vector<VkWriteDescriptorSet> writes;
};