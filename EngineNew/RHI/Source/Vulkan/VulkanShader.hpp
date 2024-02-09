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

	const InputAttributeDescription&			AttributeDescription() const { return m_attributesDescription; }

	const VkVertexInputBindingDescription&		InputDescription() const { return m_inputDescription; }

	VkShaderModule								Module(ShaderStage stage) const { return m_modules[stage]; }

	VkDescriptorSetLayout						Layout() const { return m_layout; }

	const eastl::vector<VkPushConstantRange>&	Constants() const { return m_constants; }

	virtual void								SetTexture(const std::shared_ptr<Texture>& texture, int slot) override;
	virtual void								SetBuffer(const std::shared_ptr<Buffer>& buffer,
															int slot,
															ShaderStage
															stage,
															int offset = 0) override;

	virtual void								Sync() override;

private:
	using ModuleMap = eastl::unordered_map<ShaderStage, VkShaderModule>;

	mutable ModuleMap									m_modules;
	eastl::vector<VkVertexInputAttributeDescription>	m_attributesDescription;
	eastl::vector<VkPushConstantRange>					m_constants;
	VkVertexInputBindingDescription						m_inputDescription;

	VkDescriptorSetLayout								m_layout = nullptr;
	VkDescriptorSet										m_descriptorSet = nullptr;
	// TODO: Use shared descriptor pool
	VkDescriptorPool									m_descriptorPool = nullptr;

	struct BufferInfo
	{
		std::weak_ptr<rhi::Buffer>  m_buffer;
		int                         m_slot;
		int                         m_offset;
		rhi::ShaderStage            m_stage;
	};

	struct TextureInfo
	{
		std::weak_ptr<rhi::Texture>		m_texture;
		int								m_slot;
	};

	eastl::vector<BufferInfo>	m_buffersToSync;
	eastl::vector<TextureInfo>	m_texturesToSync;

	void FillVertexData();
	void CreateDescriptorSetLayout();
	void FillPushContansts();
	void AllocateDescriptorSet();
};

}