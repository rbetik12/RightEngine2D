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

private:
	using ModuleMap = eastl::unordered_map<ShaderStage, VkShaderModule>;

	mutable ModuleMap									m_modules;
	VkDescriptorSetLayout								m_layout = nullptr;
	eastl::vector<VkVertexInputAttributeDescription>	m_attributesDescription;
	eastl::vector<VkPushConstantRange>					m_constants;
	VkVertexInputBindingDescription						m_inputDescription;

	void FillVertexData();
	void CreateDescriptorSetLayout();
	void FillPushContansts();
};

}