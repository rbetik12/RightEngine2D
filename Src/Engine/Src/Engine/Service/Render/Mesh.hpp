#pragma once

#include <Engine/Config.hpp>
#include <RHI/Device.hpp>

namespace engine::render
{

class ENGINE_API Mesh
{
public:
    Mesh(const std::shared_ptr<rhi::Buffer>& vb,
        const std::shared_ptr<rhi::Buffer>& ib = {}) : m_vertexBuffer(vb), m_indexBuffer(ib)
    {}

    const std::shared_ptr<rhi::Buffer>& VertexBuffer() { ENGINE_ASSERT(m_vertexBuffer); return m_vertexBuffer; }
    const std::shared_ptr<rhi::Buffer>& IndexBuffer() { return m_indexBuffer; }

private:
    std::shared_ptr<rhi::Buffer> m_vertexBuffer;
    std::shared_ptr<rhi::Buffer> m_indexBuffer;
};

} // engine::render