#include <RHI/ShaderCompiler.hpp>
#include "Vulkan/VulkanShaderCompiler.hpp"

namespace rhi
{
	std::shared_ptr<ShaderCompiler> ShaderCompiler::Create(Options options)
	{
		return std::make_shared<rhi::vulkan::VulkanShaderCompiler>(options);
	}
}
