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

	private:
		VkShaderModule										m_module;

		// Only for vertex shader
		eastl::vector<VkVertexInputAttributeDescription>	m_vertexBufferAttributesDescription;
		VkVertexInputBindingDescription						m_vertexBufferDescription;

		void fillVertexData();
	};

}