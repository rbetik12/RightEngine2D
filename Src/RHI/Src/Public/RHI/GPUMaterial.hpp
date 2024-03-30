#pragma once

#include <RHI/Config.hpp>
#include <RHI/Device.hpp>

namespace rhi
{

class RHI_API GPUMaterial
{
public:
    virtual ~GPUMaterial() = default;

    virtual void SetTexture(const std::shared_ptr<Texture>& texture, uint8_t slot) = 0;
    virtual void SetBuffer(const std::shared_ptr<Buffer>& buffer,
                                        uint8_t slot,
                                        ShaderStage stage,
                                        int offset = 0) = 0;

    virtual void Sync() = 0;

protected:
    bool m_dirty = false;
};

} // rhi