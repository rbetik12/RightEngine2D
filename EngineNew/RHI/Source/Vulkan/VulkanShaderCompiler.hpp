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

	virtual CompiledShaderData Compile(std::string_view path, ShaderType type) override;

private:
	using ReflectionMap = eastl::unordered_map<ShaderStage, ShaderReflection>;
	using ShaderMap = eastl::unordered_map<ShaderStage, std::string>;

	struct Context
	{
		std::string_view	m_path;
		ShaderMap			m_stageCodeStr;
		ShaderType			m_type;
	};

	void							ReadShader(const std::string& text, Context& ctx) const;
	std::string						ReadShader(std::string_view path);
	void							PreprocessShader(Context& ctx);
	ShaderReflection				MergeReflection(const ReflectionMap& reflectionMap, std::string_view path);
	[[nodiscard]] ShaderReflection	ReflectShader(const core::Blob& shaderBlob, std::string_view path, ShaderStage stage);
	[[nodiscard]] core::Blob		CompileShader(const std::string& shaderCode, std::string_view path, ShaderStage stage);

	eastl::unordered_map<std::string, std::string>	m_includeCache;
	std::mutex										m_includeCacheMutex;
	std::mutex										m_glslangMutex;
};

}
