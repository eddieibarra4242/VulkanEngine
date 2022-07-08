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
#include <glm/glm.hpp>
#include <array>

#include "Buffer.hpp"

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

class Mesh
{
  public:
    Mesh(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    void record_draw_command(const VkCommandBuffer& commandBuffer, uint32_t instanceCount = 1, uint32_t instanceIDOffset = 0) const;

    DELETE_COPY(Mesh);

    void operator=(Mesh&& other);
    Mesh(Mesh&& other);

  private:
    Buffer m_vertexBuf;
    Buffer m_indexBuf;

    uint32_t m_drawCount;
};

class Model
{
  public:
    Model() = default;

    Model(
        const std::vector<uint32_t>& indices,
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec2>& texCoords,
        const std::vector<glm::vec3>& normals = std::vector<glm::vec3>(),
        const std::vector<glm::vec3>& tangents = std::vector<glm::vec3>()) : m_indices(indices),
                                                                             m_positions(positions),
                                                                             m_texCoords(texCoords),
                                                                             m_normals(normals),
                                                                             m_tangents(tangents) {}

    constexpr bool isValid() const
    {
        return m_positions.size() != 0 && m_indices.size() != 0;
    }

    constexpr bool isComplete() const
    {
        size_t necessarySize = m_vertices.size();
        return m_positions.size() == necessarySize && m_texCoords.size() == necessarySize && m_normals.size() == necessarySize && m_tangents.size() == necessarySize;
    }

    inline uint32_t addVertex(glm::vec3 position)
    {
        uint32_t ret = static_cast<uint32_t>(m_positions.size());
        m_positions.push_back(position);
        return ret;
    }

    inline uint32_t addVertex(glm::vec3 position, glm::vec2 texCoord)
    {
        uint32_t ret = static_cast<uint32_t>(m_positions.size());
        m_positions.push_back(position);
        m_texCoords.push_back(texCoord);
        return ret;
    }

    inline uint32_t addVertex(glm::vec3 position, glm::vec2 texCoord, glm::vec3 normal)
    {
        uint32_t ret = static_cast<uint32_t>(m_positions.size());
        m_positions.push_back(position);
        m_texCoords.push_back(texCoord);
        m_normals.push_back(normal);
        return ret;
    }

    inline uint32_t addVertex(glm::vec3 position, glm::vec2 texCoord, glm::vec3 normal, glm::vec3 tangent)
    {
        uint32_t ret = static_cast<uint32_t>(m_positions.size());
        m_positions.push_back(position);
        m_texCoords.push_back(texCoord);
        m_normals.push_back(normal);
        m_tangents.push_back(tangent);
        return ret;
    }

    inline void addFace(uint32_t i0, uint32_t i1, uint32_t i2)
    {
        m_indices.push_back(i0);
        m_indices.push_back(i1);
        m_indices.push_back(i2);
    }

    void calcNormals();
    void calcTangents();
    Model& finalize();

    constexpr const std::vector<Vertex>& getVertices() const { return m_vertices; }
    constexpr const std::vector<uint32_t>& getIndices() const { return m_indices; }

  private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec2> m_texCoords;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec3> m_tangents;
};