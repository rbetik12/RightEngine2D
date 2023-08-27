#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderCompiler.hpp>
#include <Core/EASTLIntergration.hpp>
#include <EASTL/unordered_map.h>
#include <glslang/Include/glslang_c_shader_types.h>
#include <mutex>

namespace rhi::vulkan
{
	class RHI_API VulkanShaderCompiler : public ShaderCompiler
	{
	public:
		VulkanShaderCompiler(Options options);

		virtual ~VulkanShaderCompiler() override;

		virtual ShaderData Compile(std::string_view path) override;

	private:
		struct Context
		{
			std::string_view m_path;
			std::string_view m_rawCodeStr;
			std::string_view m_processedCodeStr;
			rhi::ShaderType  m_type;
		};

		std::string			ReadShader(std::string_view path) const;
		std::string			PreprocessShader(const Context& ctx);
		core::Blob			CompileShader(const Context& ctx);

		eastl::unordered_map<std::string, std::string>	m_includeCache;
		std::mutex										m_includeCacheMutex;
	};
}
