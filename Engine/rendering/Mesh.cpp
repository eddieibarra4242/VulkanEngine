#include "Mesh.hpp"

Mesh::Mesh(const Device &device, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) : 
    m_vertexBuf { device, vertices.size() * sizeof(Vertex), 1, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT }, 
    m_indexBuf { device, indices.size() * sizeof(uint32_t), 1, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT }, 
    m_drawCount { static_cast<uint32_t>(indices.size()) }
{
    m_vertexBuf.write(vertices.data(), vertices.size() * sizeof(Vertex), 0);
    m_indexBuf.write(indices.data(), indices.size() * sizeof(uint32_t), 0);
}

void Mesh::record_draw_command(const VkCommandBuffer &commandBuffer, uint32_t instanceCount, uint32_t instanceIDOffset) const
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuf.buffer(), &offset);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuf.buffer(), offset, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, m_drawCount, instanceCount, 0, 0, instanceIDOffset);
}

void Mesh::operator=(Mesh&& other)
{
    m_vertexBuf = std::move(other.m_vertexBuf);
    m_indexBuf = std::move(other.m_indexBuf);
    m_drawCount = other.m_drawCount;
}

Mesh::Mesh(Mesh&& other) : m_vertexBuf(std::move(other.m_vertexBuf)), m_indexBuf(std::move(other.m_indexBuf)), m_drawCount{ other.m_drawCount }
{

}

void Model::calcNormals()
{
    m_normals.clear();
    m_normals.reserve(m_positions.size());

    for (uint32_t i = 0; i < m_positions.size(); i++)
        m_normals.emplace_back(0, 0, 0);

    for (uint32_t i = 0; i < m_indices.size(); i += 3) {
        uint32_t i0 = m_indices[i];
        uint32_t i1 = m_indices[i + 1];
        uint32_t i2 = m_indices[i + 2];

        glm::vec3 v1 = m_positions[i1] - m_positions[i0];
        glm::vec3 v2 = m_positions[i2] - m_positions[i0];

        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

        m_normals[i0] += normal;
        m_normals[i1] += normal;
        m_normals[i2] += normal;
    }

    for (uint32_t i = 0; i < m_normals.size(); i++)
        m_normals[i] = glm::normalize(m_normals[i]);
}

void Model::calcTangents()
{
    m_tangents.clear();
    m_tangents.reserve(m_positions.size());

    for (uint32_t i = 0; i < m_positions.size(); i++)
        m_tangents.emplace_back(0, 0, 0);

    for (uint32_t i = 0; i < m_indices.size(); i += 3) {
        uint32_t i0 = m_indices[i];
        uint32_t i1 = m_indices[i + 1];
        uint32_t i2 = m_indices[i + 2];

        glm::vec3 edge1 = m_positions[i1] - m_positions[i0];
        glm::vec3 edge2 = m_positions[i2] - m_positions[i0];

        float deltaU1 = m_texCoords[i1].x - m_texCoords[i0].x;
        float deltaU2 = m_texCoords[i2].x - m_texCoords[i0].x;
        float deltaV1 = m_texCoords[i1].y - m_texCoords[i0].y;
        float deltaV2 = m_texCoords[i2].y - m_texCoords[i0].y;

        float dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
        float f = dividend == 0.0f ? 0.0f : 1.0f / dividend;

        glm::vec3 tangent = glm::vec3(0, 0, 0);

        tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
        tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
        tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

        m_tangents[i0] += tangent;
        m_tangents[i1] += tangent;
        m_tangents[i2] += tangent;
    }

    for (uint32_t i = 0; i < m_tangents.size(); i++)
        m_tangents[i] = glm::normalize(m_tangents[i]);
}

Model& Model::finalize()
{
    if (isComplete()) {
        return *this;
    }

    size_t necessarySize = m_positions.size();

    if (m_texCoords.size() < necessarySize) {
        for (size_t i = m_texCoords.size(); i < m_positions.size(); i++) {
            m_texCoords.emplace_back(0.0f, 0.0f);
        }
    }

    if (m_normals.size() != necessarySize) {
        calcNormals();
    }

    if (m_tangents.size() != necessarySize) {
        calcTangents();
    }

    for(size_t i = 0; i < necessarySize; i++) {
        m_vertices.emplace_back(Vertex{ m_positions[i]/*, m_texCoords[i], m_normals[i], m_tangents[i]*/ });
    }

    return *this;
}