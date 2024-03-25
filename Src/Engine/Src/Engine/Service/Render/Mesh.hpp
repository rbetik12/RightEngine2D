#pragma once

#include <Engine/Config.hpp>
#include <RHI/Device.hpp>

namespace engine::render
{

class ENGINE_API SubMesh
{
public:
    SubMesh(const std::shared_ptr<rhi::Buffer>& vb, const std::shared_ptr<rhi::Buffer>& ib = {}) : m_vertexBuffer(vb), m_indexBuffer(ib)
    {}

    const std::shared_ptr<rhi::Buffer>& VertexBuffer() { ENGINE_ASSERT(m_vertexBuffer); return m_vertexBuffer; }
    const std::shared_ptr<rhi::Buffer>& IndexBuffer() { return m_indexBuffer; }

private:
    std::shared_ptr<rhi::Buffer> m_vertexBuffer;
    std::shared_ptr<rhi::Buffer> m_indexBuffer;
};

class ENGINE_API Mesh
{
public:
    using SubMeshList = eastl::vector<std::shared_ptr<SubMesh>>;

    const std::shared_ptr<SubMesh>& GetSubMesh(uint32_t index) const { ENGINE_ASSERT(index < m_submeshes.size()); return m_submeshes[index]; }

    const SubMeshList& GetSubMeshList() const { return m_submeshes; }

    void AddSubMesh(const std::shared_ptr<SubMesh>& submesh);

private:
    eastl::vector<std::shared_ptr<SubMesh>> m_submeshes;
};

} // engine::render