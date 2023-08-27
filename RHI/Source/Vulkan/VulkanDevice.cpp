#include "VulkanDevice.hpp"
#include "VulkanShaderCompiler.hpp"

namespace rhi::vulkan
{

	VulkanDevice::~VulkanDevice()
	{}

	std::shared_ptr<ShaderCompiler> VulkanDevice::CreateShaderCompiler(const ShaderCompiler::Options& options)
	{
		return std::make_shared<VulkanShaderCompiler>(options);
	}
}
