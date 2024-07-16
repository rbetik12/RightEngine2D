#pragma once

#include <RHI/GPUMaterial.hpp>
#include "VulkanShader.hpp"

namespace rhi::vulkan
{

class RHI_API VulkanGPUMaterial : public GPUMaterial
{
public:
    VulkanGPUMaterial(const std::shared_ptr<VulkanShader>& shader);
    virtual ~VulkanGPUMaterial() override;

    VkDescriptorSet     DescriptorSet() const { return m_descriptorSet; }

    virtual void        SetTexture(const std::shared_ptr<Texture>& texture, uint8_t slot, uint8_t mipLevel = 0) override;
    virtual void        SetBuffer(const std::shared_ptr<Buffer>& buffer,
        uint8_t slot,
        ShaderStage stage,
        int offset = 0) override;

    virtual void        Sync() override;

    static void         Destroy();

private:
    static void         AllocateDescriptorPool();
    VkDescriptorType    DescriptorType(uint8_t slot);

    struct BufferInfo
    {
        std::weak_ptr<rhi::Buffer>  m_buffer;
        uint32_t                    m_offset = 0;
        uint8_t                     m_slot = 0;
        rhi::ShaderStage            m_stage = ShaderStage::NONE;
    };

    struct TextureInfo
    {
        std::weak_ptr<rhi::Texture> m_texture;
        uint8_t                     m_slot = 0;
        uint8_t                     m_mipLevel = 0;
    };

    eastl::vector<BufferInfo>           m_buffersToSync;
    eastl::vector<TextureInfo>          m_texturesToSync;

    const ShaderDescriptor*             m_shaderDesc = nullptr;
    VkDescriptorSet                     m_descriptorSet = nullptr;

    inline static VkDescriptorPool      s_descriptorPool = nullptr;
};

} // rhi::vulkan