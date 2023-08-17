#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderCompiler.hpp>
#include <EASTL/unordered_map.h>
#include <mutex>

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
		std::string PreprocessShader(std::string_view shaderCode, std::string_view path) const;

		eastl::unordered_map<std::string, std::string>	m_includeCache;
		std::mutex										m_includeCacheMutex;
	};
}
