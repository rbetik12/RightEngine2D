#pragma once

#include <RHI/Config.hpp>
#include <RHI/RenderPassDescriptor.hpp>

namespace rhi
{

class RHI_API RenderPass
{
public:
    virtual ~RenderPass() = default;

    bool                        HasDepth() const { return m_descriptor.m_depthStencilAttachment.m_texture != nullptr; }

    const RenderPassDescriptor& Descriptor() const { return m_descriptor; }

protected:
    RenderPassDescriptor m_descriptor;

    RenderPass(const RenderPassDescriptor& descriptor) : m_descriptor(descriptor)
    {}
};

}