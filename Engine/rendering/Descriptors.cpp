#include "Descriptors.hpp"

#include "VulkanUtils.hpp"

void DescriptorSetLayout::destroy_internal()
{
    if(m_seal && m_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_parent_dev.device(), m_layout, nullptr);
    }
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    destroy_internal();
}

DescriptorSetLayout& DescriptorSetLayout::addBinding(uint32_t binding, VkDescriptorType descType, VkShaderStageFlags stageFlags, uint32_t count)
{
    if(m_seal) {
        return *this;
    }

    if(m_bindings.count(binding) != 0) {
        spdlog::error("Binding {} is already in use!", binding);
        return *this;
    }

    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    m_bindings[binding] = layoutBinding;
    return *this;
}

void DescriptorSetLayout::seal()
{
    if(m_seal) {
        return;
    }

    m_seal = true;

    std::vector<VkDescriptorSetLayoutBinding> values;
    for(auto kv : m_bindings) {
        values.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo dslCreateInfo{ };
    dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslCreateInfo.bindingCount = static_cast<uint32_t>(values.size());
    dslCreateInfo.pBindings = values.data();

    ASSERT_VK_SUCCESS(vkCreateDescriptorSetLayout(m_parent_dev.device(), &dslCreateInfo, nullptr, &m_layout), "Failed to allocate Descriptor Set Layout!");
}

void DescriptorSetLayout::operator=(DescriptorSetLayout&& other)
{
    destroy_internal();

    this->m_layout = other.m_layout;
    this->m_seal = other.m_seal;
    this->m_bindings = std::move(other.m_bindings);

    other.m_layout = VK_NULL_HANDLE;
    other.m_seal = false;
}

void DescriptorPool::destroy_internal()
{
    if(m_seal && m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_parent_dev.device(), m_descriptorPool, nullptr);
    }
}

DescriptorPool::~DescriptorPool()
{
    destroy_internal();
}

DescriptorPool& DescriptorPool::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
    if(!m_seal) {
        m_poolSizes.push_back({descriptorType, count});
    }
    
    return *this;
}

DescriptorPool& DescriptorPool::setPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    if(!m_seal) {
        m_poolFlags = flags;
    }

    return *this;
}

DescriptorPool& DescriptorPool::setMaxSets(uint32_t count)
{
    if(!m_seal) {
        m_maxSets = count;
    }

    return *this;
}

void DescriptorPool::seal()
{
    if(m_seal) {
        return;
    }

    m_seal = true;

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
    descriptorPoolInfo.pPoolSizes = m_poolSizes.data();
    descriptorPoolInfo.maxSets = m_maxSets;
    descriptorPoolInfo.flags = m_poolFlags;

    ASSERT_VK_SUCCESS(vkCreateDescriptorPool(m_parent_dev.device(), &descriptorPoolInfo, nullptr, &m_descriptorPool), "Failed to create Descriptor Pool!");
}

bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const 
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(m_parent_dev.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }

    return true;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const 
{
    vkFreeDescriptorSets(
        m_parent_dev.device(),
        m_descriptorPool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
}

void DescriptorPool::reset() 
{
    vkResetDescriptorPool(m_parent_dev.device(), m_descriptorPool, 0);
}

void DescriptorPool::operator=(DescriptorPool&& other)
{
    destroy_internal();

    this->m_poolSizes = std::move(other.m_poolSizes);
    this->m_maxSets = other.m_maxSets;
    this->m_poolFlags = other.m_poolFlags;
    this->m_descriptorPool = other.m_descriptorPool;
    this->m_seal = other.m_seal;

    other.m_descriptorPool = VK_NULL_HANDLE;
    other.m_seal = false;
}

DescriptorSetWriter& DescriptorSetWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* info)
{
    if(m_layout.m_bindings.count(binding) != 1) {
        spdlog::error("Binding {} does not exist in layout!", binding);
        return *this;
    }

    VkDescriptorSetLayoutBinding& bindingDesc = m_layout.m_bindings[binding];

    if(bindingDesc.descriptorCount != 1) {
        spdlog::error("Binding {} does not 1 descriptor. Has {}.", binding, bindingDesc.descriptorCount);
        return *this;
    }

    VkWriteDescriptorSet wr{ };
    wr.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wr.descriptorType = bindingDesc.descriptorType;
    wr.dstBinding = binding;
    wr.pBufferInfo = info;
    wr.pImageInfo = nullptr;
    wr.descriptorCount = 1;

    writes.push_back(wr);
    return *this;
}

DescriptorSetWriter& DescriptorSetWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* info)
{
    if(m_layout.m_bindings.count(binding) != 1) {
        spdlog::error("Binding {} does not exist in layout!", binding);
        return *this;
    }

    VkDescriptorSetLayoutBinding& bindingDesc = m_layout.m_bindings[binding];

    if(bindingDesc.descriptorCount != 1) {
        spdlog::error("Binding {} does not 1 descriptor. Has {}.", binding, bindingDesc.descriptorCount);
        return *this;
    }

    VkWriteDescriptorSet wr{ };
    wr.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wr.descriptorType = bindingDesc.descriptorType;
    wr.dstBinding = binding;
    wr.pBufferInfo = nullptr;
    wr.pImageInfo = info;
    wr.descriptorCount = 1;

    writes.push_back(wr);
    return *this;
}

bool DescriptorSetWriter::createAndWrite(VkDescriptorSet& set)
{
    bool success = m_pool.allocateDescriptor(m_layout.m_layout, set);

    if(success) {
        write(set);
    }

    return success;
}

void DescriptorSetWriter::write(VkDescriptorSet& set)
{
    for (auto& wr : writes) {
        wr.dstSet = set;
    }

    vkUpdateDescriptorSets(m_layout.m_parent_dev.device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}