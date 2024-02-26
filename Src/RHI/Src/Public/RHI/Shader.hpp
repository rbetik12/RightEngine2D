#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderDescriptor.hpp>
#include <RHI/Device.hpp>

namespace rhi
{

class RHI_API Shader
{
public:
    virtual ~Shader() = default;

    const ShaderDescriptor& Descriptor() const { return m_descriptor; }

    virtual void            SetTexture(const std::shared_ptr<Texture>& texture, int slot) = 0;
    virtual void            SetBuffer(const std::shared_ptr<Buffer>& buffer,
                                        int slot,
                                        ShaderStage stage,
                                        int offset = 0) = 0;

    virtual void            Sync() = 0;

protected:
    ShaderDescriptor    m_descriptor;
    bool                m_dirty = false;

    Shader(const ShaderDescriptor& descriptor) : m_descriptor(descriptor)
    {}
};

}
