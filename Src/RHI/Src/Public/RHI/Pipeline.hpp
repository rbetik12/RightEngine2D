#pragma once

#include <RHI/Config.hpp>
#include <RHI/Shader.hpp>
#include <RHI/Buffer.hpp>
#include <RHI/PipelineDescriptor.hpp>
#include <cstdint>

namespace rhi
{

class RHI_API Pipeline
{
public:
    virtual ~Pipeline() = default;

    const PipelineDescriptor& Descriptor() const { return m_descriptor; }

    inline uint32_t VertexCount(const std::shared_ptr<rhi::Buffer>& buffer) const
    {
        const auto& desc = buffer->Descriptor();
        RHI_ASSERT(desc.m_type == BufferType::VERTEX);
        return desc.m_size / m_descriptor.m_shader->Descriptor().m_reflection.m_inputLayout.Stride();
    }

protected:
    PipelineDescriptor m_descriptor;

    Pipeline(const PipelineDescriptor& descriptor) : m_descriptor(descriptor)
    {
        RHI_ASSERT(descriptor.m_shader);
    }
};

}