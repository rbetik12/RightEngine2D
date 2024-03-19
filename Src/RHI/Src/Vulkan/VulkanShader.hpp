#pragma once

#include <RHI/Config.hpp>
#include <RHI/Shader.hpp>

namespace rhi::vulkan
{

class RHI_API VulkanShader : public Shader
{
public:
    VulkanShader(const ShaderDescriptor& descriptor);

    virtual ~VulkanShader() override;

    using InputAttributeDescription = eastl::vector<VkVertexInputAttributeDescription>;

    const InputAttributeDescription&           AttributeDescription() const { return m_attributesDescription; }

    const VkVertexInputBindingDescription&     InputDescription() const { return m_inputDescription; }

    VkShaderModule                             Module(ShaderStage stage) const { return m_modules[stage]; }

    VkDescriptorSetLayout                      Layout() const { return m_layout; }
    VkDescriptorSet                            DesciptorSet() const { return m_descriptorSet; }

    const eastl::vector<VkPushConstantRange>&  Constants() const { return m_constants; }

    virtual void                               SetTexture(const std::shared_ptr<Texture>& texture, uint8_t slot) override;
    virtual void                               SetBuffer(const std::shared_ptr<Buffer>& buffer,
                                                            uint8_t slot,
                                                            ShaderStage
                                                            stage,
                                                            int offset = 0) override;

    virtual void                               Sync() override;

private:
    using ModuleMap = eastl::unordered_map<ShaderStage, VkShaderModule>;

    mutable ModuleMap                                   m_modules;
    eastl::vector<VkVertexInputAttributeDescription>    m_attributesDescription;
    eastl::vector<VkPushConstantRange>                  m_constants;
    VkVertexInputBindingDescription                     m_inputDescription;

    VkDescriptorSetLayout                               m_layout = nullptr;
    VkDescriptorSet                                     m_descriptorSet = nullptr;
    // TODO: Use shared descriptor pool
    VkDescriptorPool                                    m_descriptorPool = nullptr;

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
    };

    eastl::vector<BufferInfo>    m_buffersToSync;
    eastl::vector<TextureInfo>   m_texturesToSync;

    void                FillVertexData();
    void                CreateDescriptorSetLayout();
    void                FillPushContansts();
    void                AllocateDescriptorSet();
    VkDescriptorType    DescriptorType(uint8_t slot);
};

}