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

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#include "../utils.hpp"

struct PhysicalDevice
{
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    std::vector<VkExtensionProperties> m_availableExtensions;
    VkPhysicalDeviceProperties m_properties;
    VkPhysicalDeviceFeatures2 m_features;

    std::optional<uint32_t> m_graphicsFamily;
    std::optional<uint32_t> m_presentFamily;
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR> m_presentModes;
};

class Device
{
  public:
    Device(const VkInstance& context, const VkSurfaceKHR& surface, const VkPhysicalDeviceFeatures& targetFeatures);
    ~Device();

    [[nodiscard]] constexpr const PhysicalDevice& physicalDevice() const { return m_physDevice; }
    [[nodiscard]] constexpr const VkDevice& device() const { return m_logicalDevice; }
    [[nodiscard]] constexpr const VkQueue& graphicsQueue() const { return m_graphicsQueue; }
    [[nodiscard]] constexpr const VkQueue& presentQueue() const { return m_presentQueue; }
    [[nodiscard]] constexpr const VkCommandPool& graphicsPool() const { return m_graphicsPool; }

    [[nodiscard]] constexpr const VkPhysicalDeviceProperties& properties() const { return m_physDevice.m_properties; }

    DELETE_COPY_AND_MOVE(Device);

  private:
    PhysicalDevice m_physDevice;

    VkDevice m_logicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkCommandPool m_graphicsPool;
};