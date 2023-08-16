#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderCompiler.hpp>

namespace rhi::vulkan
{
	class RHI_API VulkanShaderCompiler : public ShaderCompiler
	{
	public:
		VulkanShaderCompiler(Options options) : ShaderCompiler(options)
		{}

		virtual ShaderData Compile(std::string_view path) override;

	private:
		std::string ReadShader(std::string_view path) const;
	};
}
