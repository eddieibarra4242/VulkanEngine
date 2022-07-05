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

#include "SwapChain.hpp"

#include "VulkanUtils.hpp"

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    spdlog::warn("VK_FORMAT_B8G8R8A8_SRGB not supported! Using first available: {}", formats[0].format);
    spdlog::warn("VK_COLOR_SPACE_SRGB_NONLINEAR_KHR not supported! Using first available: {}", formats[0].colorSpace);

    return formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            return availablePresentMode;
        }
    }

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D chooseSwapExtent(const Device& device, const VkSurfaceKHR& surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice().m_physicalDevice, surface, &capabilities);

    spdlog::debug("SwapChain Extent: current: {} {} min: {} {} max: {} {}",
        capabilities.currentExtent.width,
        capabilities.currentExtent.height,
        capabilities.minImageExtent.width,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.width,
        capabilities.maxImageExtent.height);

    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        return capabilities.minImageExtent;
    }
}

void SwapChain::create()
{
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(m_parent_dev.physicalDevice().m_formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(m_parent_dev.physicalDevice().m_presentModes);
    m_extent = chooseSwapExtent(m_parent_dev, m_surface);

    uint32_t imageCount = m_parent_dev.physicalDevice().m_capabilities.minImageCount + 1;
    if (m_parent_dev.physicalDevice().m_capabilities.maxImageCount > 0 && imageCount > m_parent_dev.physicalDevice().m_capabilities.maxImageCount) {
        imageCount = m_parent_dev.physicalDevice().m_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = m_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;// use VK_IMAGE_USAGE_TRANSFER_DST_BIT for resolving later on
    createInfo.preTransform = m_parent_dev.physicalDevice().m_capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    uint32_t queueFamilyIndices[] = { m_parent_dev.physicalDevice().m_graphicsFamily.value(), m_parent_dev.physicalDevice().m_presentFamily.value() };
    if (m_parent_dev.physicalDevice().m_graphicsFamily != m_parent_dev.physicalDevice().m_presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;// Optional
        createInfo.pQueueFamilyIndices = nullptr;// Optional
    }

    ASSERT_VK_SUCCESS(vkCreateSwapchainKHR(m_parent_dev.device(), &createInfo, nullptr, &m_swapChain), "failed to create swap chain!");

    vkGetSwapchainImagesKHR(m_parent_dev.device(), m_swapChain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_parent_dev.device(), m_swapChain, &imageCount, m_images.data());
    m_imageFormat = surfaceFormat.format;

    m_imageViews.resize(m_images.size());

    for (size_t i = 0; i < m_images.size(); i++) {
        m_imageViews[i] = createImageView(m_parent_dev.device(), m_images[i], m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    m_depthImage = std::make_unique<Image>(m_parent_dev, m_extent, m_parent_dev.getDepthFormat());
}

void SwapChain::destroy()
{
    vkDeviceWaitIdle(m_parent_dev.device());

    for (auto imageView : m_imageViews) {
        vkDestroyImageView(m_parent_dev.device(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_parent_dev.device(), m_swapChain, nullptr);
}

void SwapChain::rebuild()
{
    destroy();
    create();
}

SwapChain::SwapChain(const VkSurfaceKHR& surface, Device& device) : m_parent_dev{ device }, m_surface{ surface }
{
    create();
}

SwapChain::~SwapChain()
{
    destroy();
}