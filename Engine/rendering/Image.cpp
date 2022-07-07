#include "Image.hpp"

#define VULKAN_UTILS_IMPL
#include "VulkanUtils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Buffer.hpp"

static constexpr size_t PIXEL_SIZE = 4;

void createImage(const Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ASSERT_VK_SUCCESS(vkCreateImage(device.device(), &imageInfo, nullptr, &image), "failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device.device(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, properties);

    ASSERT_VK_SUCCESS(vkAllocateMemory(device.device(), &allocInfo, nullptr, &imageMemory), "failed to allocate image memory!");

    vkBindImageMemory(device.device(), image, imageMemory, 0);
}

void copyBufferToImage(VkDevice device, VkCommandPool graphicsPool, VkQueue submitQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, graphicsPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(device, submitQueue, graphicsPool, commandBuffer);
}

void transitionImageLayout(const Device& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device.device(), device.graphicsPool());

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (Image::hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        spdlog::critical("Invalid Arguments: unsupported layout transition!");
        throw std::invalid_argument("Bad Transition");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(device.device(), device.graphicsQueue(), device.graphicsPool(), commandBuffer);
}

void destroyImage(const VkDevice& device, const VkImage& image, const VkDeviceMemory& memory, const VkImageView& imageView, const VkSampler& sampler)
{
    if (sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, sampler, nullptr);
    }

    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    if (image != VK_NULL_HANDLE) {
        vkDestroyImage(device, image, nullptr);
    }

    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory, nullptr);
    }
}

void Image::createInternal(void* data, VkFormat format)
{
    size_t imageSize = m_width * m_height * PIXEL_SIZE;

    Buffer buf{ m_device, imageSize, 1, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    buf.write(data, imageSize, 0);

    createImage(m_device, m_width, m_height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);

    transitionImageLayout(m_device, m_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(m_device.device(), m_device.graphicsPool(), m_device.graphicsQueue(), buf.buffer(), m_image, static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height));
    transitionImageLayout(m_device, m_image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_imageView = createImageView(m_device.device(), m_image, format, VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo sampInfo{};
    sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampInfo.minFilter = VK_FILTER_LINEAR;
    sampInfo.magFilter = VK_FILTER_LINEAR;
    sampInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampInfo.anisotropyEnable = VK_TRUE;
    sampInfo.maxAnisotropy = m_device.properties().limits.maxSamplerAnisotropy;
    sampInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampInfo.unnormalizedCoordinates = VK_FALSE;
    sampInfo.compareEnable = VK_FALSE;
    sampInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    sampInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampInfo.mipLodBias = 0.0f;
    sampInfo.minLod = 0.0f;
    sampInfo.maxLod = 0.0f;

    ASSERT_VK_SUCCESS(vkCreateSampler(m_device.device(), &sampInfo, nullptr, &m_sampler), "Failed to create sampler.");
}

Image::Image(const Device& device, const char* filename, VkFormat format) : m_device{ device }
{
    int w, h, c;
    stbi_uc* data = stbi_load(filename, &w, &h, &c, PIXEL_SIZE);

    m_width = static_cast<uint32_t>(w);
    m_height = static_cast<uint32_t>(h);

    if (data == nullptr) {
        spdlog::critical("Cannot load {}", filename);
        throw std::runtime_error("File not found");
    }

    createInternal(data, format);
    stbi_image_free(data);
}

Image::Image(const Device& device, uint32_t width, uint32_t height, void* data, VkFormat format) : m_device{ device }, m_width{ width }, m_height{ height }
{
    createInternal(data, format);
}

Image::Image(const Device& device, VkExtent2D extent, VkFormat depthFormat) : m_device{ device }
{
    createImage(device, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);
    m_imageView = createImageView(device.device(), m_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(device, m_image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

Image::~Image()
{
    destroyImage(m_device.device(), m_image, m_memory, m_imageView, m_sampler);
}

void Image::operator=(Image&& other)
{
    destroyImage(m_device.device(), m_image, m_memory, m_imageView, m_sampler);
    this->m_image = other.m_image;
    this->m_memory = other.m_memory;
    this->m_imageView = other.m_imageView;
    this->m_sampler = other.m_sampler;
    this->m_width = other.m_width;
    this->m_height = other.m_height;

    other.m_image = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
    other.m_imageView = VK_NULL_HANDLE;
    other.m_sampler = VK_NULL_HANDLE;
}

Image::Image(Image&& other) : m_device{ other.m_device }
{
    this->operator=(std::move(other));
}