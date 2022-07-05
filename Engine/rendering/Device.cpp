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

#include "Device.hpp"

#include <set>

#include <spdlog/spdlog.h>

#include "../window.hpp"

std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void getDeviceExtensions(PhysicalDevice& device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device.m_physicalDevice, nullptr, &extensionCount, nullptr);
    device.m_availableExtensions.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(device.m_physicalDevice, nullptr, &extensionCount, device.m_availableExtensions.data());
}

void findQueueFamilies(PhysicalDevice& device, VkSurfaceKHR surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device.m_physicalDevice, &queueFamilyCount, nullptr);
    std::unique_ptr<VkQueueFamilyProperties[]> queueFamilies = std::make_unique<VkQueueFamilyProperties[]>(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device.m_physicalDevice, &queueFamilyCount, queueFamilies.get());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device.m_physicalDevice, i, surface, &presentSupport);

        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            device.m_graphicsFamily = i;
        }

        if (presentSupport) {
            device.m_presentFamily = i;
        }

        if (device.m_graphicsFamily.has_value() && device.m_presentFamily.has_value()) {
            return;
        }
    }
}

void querySwapChainSupport(PhysicalDevice& device, VkSurfaceKHR surface)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.m_physicalDevice, surface, &device.m_capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physicalDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        device.m_formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.m_physicalDevice, surface, &formatCount, device.m_formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        device.m_presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device.m_physicalDevice, surface, &presentModeCount, device.m_presentModes.data());
    }
}

void readPhysicalDevice(VkPhysicalDevice vkDeviceIn, VkSurfaceKHR surface, PhysicalDevice& deviceOut)
{
    deviceOut.m_physicalDevice = vkDeviceIn;
    getDeviceExtensions(deviceOut);
    findQueueFamilies(deviceOut, surface);
    querySwapChainSupport(deviceOut, surface);
}

bool checkTargetedFeatures(const PhysicalDevice& physicalDevice, const VkPhysicalDeviceFeatures& targetFeatures)
{
    for (size_t i = 0; i < elemSize<VkBool32>(sizeof(VkPhysicalDeviceFeatures)); i++) {
        const VkBool32* targets = reinterpret_cast<const VkBool32*>(&targetFeatures);
        const VkBool32* supported = reinterpret_cast<const VkBool32*>(&physicalDevice.m_features.features);

        if (targets[i] == VK_FALSE) {
            continue;
        }

        if (targets[i] != supported[i]) {
            return false;
        }
    }

    return true;
}

bool checkExtensionSupport(const PhysicalDevice& physicalDevice)
{
    std::set<std::string> extensionNames;

    for (auto extension : physicalDevice.m_availableExtensions) {
        extensionNames.emplace(extension.extensionName);
    }

    for (auto devExtension : deviceExtensions) {
        if (!extensionNames.contains(devExtension)) {
            return false;
        }
    }

    return true;
}

bool isSuitable(const PhysicalDevice& physicalDevice, const VkPhysicalDeviceFeatures& targetFeatures)
{
    return checkTargetedFeatures(physicalDevice, targetFeatures) && checkExtensionSupport(physicalDevice) && physicalDevice.m_graphicsFamily.has_value() && physicalDevice.m_presentFamily.has_value() && !physicalDevice.m_formats.empty() && !physicalDevice.m_presentModes.empty();
}

Device::Device(const VkInstance& context, const VkSurfaceKHR& surface, const VkPhysicalDeviceFeatures& targetFeatures)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context, &deviceCount, nullptr);

    if (deviceCount == 0) {
        spdlog::critical("Failed to find GPUs with Vulkan support, consider using OpenGL!");
        throw std::runtime_error("No physical devices.");
    }

    std::vector<VkPhysicalDevice> devices{ deviceCount };
    vkEnumeratePhysicalDevices(context, &deviceCount, devices.data());

    for (auto physDev : devices) {
        readPhysicalDevice(physDev, surface, m_physDevice);
        if (isSuitable(m_physDevice, targetFeatures)) {
            break;
        } else {
            m_physDevice.m_physicalDevice = VK_NULL_HANDLE;
        }
    }

    if (m_physDevice.m_physicalDevice == VK_NULL_HANDLE) {
        spdlog::critical("Failed to find suitable GPU, consider using OpenGL!");
        throw std::runtime_error("No physical devices made the cut.");
    }

    std::set<uint32_t> uniqueQueueFamilies = { m_physDevice.m_graphicsFamily.value(), m_physDevice.m_presentFamily.value() };
    size_t num_queues = uniqueQueueFamilies.size();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{ num_queues };

    float queuePriority = 1.0f;
    size_t i = 0;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i++] = queueCreateInfo;
    }

    vkGetPhysicalDeviceProperties(m_physDevice.m_physicalDevice, &m_physDevice.m_properties);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(num_queues);
    createInfo.pEnabledFeatures = VK_NULL_HANDLE;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifndef NDEBUG
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif


    // FIXME: The following commented code randomly seg faults in driver.

    // m_physDevice.m_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    // vkGetPhysicalDeviceFeatures2(m_physDevice.m_physicalDevice, &m_physDevice.m_features);

    // VkPhysicalDeviceDescriptorIndexingFeatures vpddif{};
    // vpddif.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    // vpddif.runtimeDescriptorArray = VK_TRUE;
    // m_physDevice.m_features.pNext = &vpddif;

    // createInfo.pNext = &m_physDevice.m_features;

    ASSERT_VK_SUCCESS(vkCreateDevice(m_physDevice.m_physicalDevice, &createInfo, nullptr, &m_logicalDevice), "Failed to create logical device!");

    vkGetDeviceQueue(m_logicalDevice, m_physDevice.m_graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, m_physDevice.m_presentFamily.value(), 0, &m_presentQueue);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_physDevice.m_graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    ASSERT_VK_SUCCESS(vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_graphicsPool), "failed to create command pool!");
}

Device::~Device()
{
    vkDestroyCommandPool(m_logicalDevice, m_graphicsPool, nullptr);
    vkDestroyDevice(m_logicalDevice, nullptr);
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physDevice.m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return -1U;
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physDevice.m_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

VkFormat Device::getDepthFormat()
{
    if (m_depthFormat == VK_FORMAT_UNDEFINED) {
        m_depthFormat = findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    return m_depthFormat;
}