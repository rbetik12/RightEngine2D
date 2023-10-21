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
	using DescriptorSetLayout = eastl::vector<VkDescriptorSetLayoutBinding>;

	const InputAttributeDescription&		AttributeDescription() const
	{ return m_attributesDescription; }

	const VkVertexInputBindingDescription&	InputDescription() const
	{ return m_inputDescription; }

	VkShaderModule							Module(ShaderStage stage) const { return m_modules[stage]; }

	const DescriptorSetLayout&				DescriptorSetBindings() const { return m_bindings; }

private:

	using ModuleMap = eastl::unordered_map<ShaderStage, VkShaderModule>;

	mutable ModuleMap									m_modules;
	eastl::vector<VkDescriptorSetLayoutBinding>			m_bindings;
	eastl::vector<VkVertexInputAttributeDescription>	m_attributesDescription;
	VkVertexInputBindingDescription						m_inputDescription;

	void FillVertexData();
	void CreateDescriptorSetLayout();
};

}