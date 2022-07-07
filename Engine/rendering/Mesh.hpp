#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

struct Vertex
{
    glm::vec3 pos;
    // glm::vec2 texCoord;
    // glm::vec3 normal;
    // glm::vec3 tangent;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        // attributeDescriptions[1].binding = 0;
        // attributeDescriptions[1].location = 1;
        // attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        // attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        // attributeDescriptions[2].binding = 0;
        // attributeDescriptions[2].location = 2;
        // attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        // attributeDescriptions[2].offset = offsetof(Vertex, normal);

        // attributeDescriptions[3].binding = 0;
        // attributeDescriptions[3].location = 3;
        // attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        // attributeDescriptions[3].offset = offsetof(Vertex, tangent);

        return attributeDescriptions;
    }
};