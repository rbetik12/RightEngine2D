#include "VulkanShader.hpp"

namespace rhi::vulkan
{
	VulkanShader::VulkanShader(const std::shared_ptr<Device>& device, const ShaderDescriptor& descriptor) : Shader(device, descriptor)
	{

	}

	VulkanShader::~VulkanShader()
	{

	}
}
