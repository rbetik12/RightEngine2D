#include "VulkanShaderCompiler.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_cross.hpp>
#include <fstream>
#include <sstream>

#define SHADER_COMPILER_VERBOSE 1

namespace fs = std::filesystem;

namespace
{
    TBuiltInResource InitResources()
    {
        TBuiltInResource Resources;

        Resources.maxLights = 32;
        Resources.maxClipPlanes = 6;
        Resources.maxTextureUnits = 32;
        Resources.maxTextureCoords = 32;
        Resources.maxVertexAttribs = 64;
        Resources.maxVertexUniformComponents = 4096;
        Resources.maxVaryingFloats = 64;
        Resources.maxVertexTextureImageUnits = 32;
        Resources.maxCombinedTextureImageUnits = 80;
        Resources.maxTextureImageUnits = 32;
        Resources.maxFragmentUniformComponents = 4096;
        Resources.maxDrawBuffers = 32;
        Resources.maxVertexUniformVectors = 128;
        Resources.maxVaryingVectors = 8;
        Resources.maxFragmentUniformVectors = 16;
        Resources.maxVertexOutputVectors = 16;
        Resources.maxFragmentInputVectors = 15;
        Resources.minProgramTexelOffset = -8;
        Resources.maxProgramTexelOffset = 7;
        Resources.maxClipDistances = 8;
        Resources.maxComputeWorkGroupCountX = 65535;
        Resources.maxComputeWorkGroupCountY = 65535;
        Resources.maxComputeWorkGroupCountZ = 65535;
        Resources.maxComputeWorkGroupSizeX = 1024;
        Resources.maxComputeWorkGroupSizeY = 1024;
        Resources.maxComputeWorkGroupSizeZ = 64;
        Resources.maxComputeUniformComponents = 1024;
        Resources.maxComputeTextureImageUnits = 16;
        Resources.maxComputeImageUniforms = 8;
        Resources.maxComputeAtomicCounters = 8;
        Resources.maxComputeAtomicCounterBuffers = 1;
        Resources.maxVaryingComponents = 60;
        Resources.maxVertexOutputComponents = 64;
        Resources.maxGeometryInputComponents = 64;
        Resources.maxGeometryOutputComponents = 128;
        Resources.maxFragmentInputComponents = 128;
        Resources.maxImageUnits = 8;
        Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
        Resources.maxCombinedShaderOutputResources = 8;
        Resources.maxImageSamples = 0;
        Resources.maxVertexImageUniforms = 0;
        Resources.maxTessControlImageUniforms = 0;
        Resources.maxTessEvaluationImageUniforms = 0;
        Resources.maxGeometryImageUniforms = 0;
        Resources.maxFragmentImageUniforms = 8;
        Resources.maxCombinedImageUniforms = 8;
        Resources.maxGeometryTextureImageUnits = 16;
        Resources.maxGeometryOutputVertices = 256;
        Resources.maxGeometryTotalOutputComponents = 1024;
        Resources.maxGeometryUniformComponents = 1024;
        Resources.maxGeometryVaryingComponents = 64;
        Resources.maxTessControlInputComponents = 128;
        Resources.maxTessControlOutputComponents = 128;
        Resources.maxTessControlTextureImageUnits = 16;
        Resources.maxTessControlUniformComponents = 1024;
        Resources.maxTessControlTotalOutputComponents = 4096;
        Resources.maxTessEvaluationInputComponents = 128;
        Resources.maxTessEvaluationOutputComponents = 128;
        Resources.maxTessEvaluationTextureImageUnits = 16;
        Resources.maxTessEvaluationUniformComponents = 1024;
        Resources.maxTessPatchComponents = 120;
        Resources.maxPatchVertices = 32;
        Resources.maxTessGenLevel = 64;
        Resources.maxViewports = 16;
        Resources.maxVertexAtomicCounters = 0;
        Resources.maxTessControlAtomicCounters = 0;
        Resources.maxTessEvaluationAtomicCounters = 0;
        Resources.maxGeometryAtomicCounters = 0;
        Resources.maxFragmentAtomicCounters = 8;
        Resources.maxCombinedAtomicCounters = 8;
        Resources.maxAtomicCounterBindings = 1;
        Resources.maxVertexAtomicCounterBuffers = 0;
        Resources.maxTessControlAtomicCounterBuffers = 0;
        Resources.maxTessEvaluationAtomicCounterBuffers = 0;
        Resources.maxGeometryAtomicCounterBuffers = 0;
        Resources.maxFragmentAtomicCounterBuffers = 1;
        Resources.maxCombinedAtomicCounterBuffers = 1;
        Resources.maxAtomicCounterBufferSize = 16384;
        Resources.maxTransformFeedbackBuffers = 4;
        Resources.maxTransformFeedbackInterleavedComponents = 64;
        Resources.maxCullDistances = 8;
        Resources.maxCombinedClipAndCullDistances = 8;
        Resources.maxSamples = 4;
        Resources.maxMeshOutputVerticesNV = 256;
        Resources.maxMeshOutputPrimitivesNV = 512;
        Resources.maxMeshWorkGroupSizeX_NV = 32;
        Resources.maxMeshWorkGroupSizeY_NV = 1;
        Resources.maxMeshWorkGroupSizeZ_NV = 1;
        Resources.maxTaskWorkGroupSizeX_NV = 32;
        Resources.maxTaskWorkGroupSizeY_NV = 1;
        Resources.maxTaskWorkGroupSizeZ_NV = 1;
        Resources.maxMeshViewCountNV = 4;

        Resources.limits.nonInductiveForLoops = 1;
        Resources.limits.whileLoops = 1;
        Resources.limits.doWhileLoops = 1;
        Resources.limits.generalUniformIndexing = 1;
        Resources.limits.generalAttributeMatrixVectorIndexing = 1;
        Resources.limits.generalVaryingIndexing = 1;
        Resources.limits.generalSamplerIndexing = 1;
        Resources.limits.generalVariableIndexing = 1;
        Resources.limits.generalConstantMatrixVectorIndexing = 1;

        return Resources;
    }

    rhi::ShaderType TypeByExtension(std::string_view path)
    {
        const auto ext = fs::path(path).extension().generic_u8string();

        if (ext == ".vert")
        {
            return rhi::ShaderType::VERTEX;
        }
        if (ext == ".frag")
        {
            return rhi::ShaderType::FRAGMENT;
        }
        RHI_ASSERT(false);
        return rhi::ShaderType::NONE;
    }

    glslang_stage_t RHITypeToGLSLangType(rhi::ShaderType type)
    {
	    switch (type)
	    {
			case rhi::ShaderType::VERTEX: return GLSLANG_STAGE_VERTEX;
			case rhi::ShaderType::FRAGMENT: return GLSLANG_STAGE_FRAGMENT;
		    default:
	        {
	            RHI_ASSERT(false);
	            return static_cast<glslang_stage_t>(-1);
	        }
	    }
    }

    EShLanguage RHITypeToEShLanguageType(rhi::ShaderType type)
    {
        switch (type)
        {
	        case rhi::ShaderType::VERTEX: return EShLangVertex;
	        case rhi::ShaderType::FRAGMENT: return EShLangFragment;
	        default:
	        {
	            RHI_ASSERT(false);
	            return static_cast<EShLanguage>(-1);
	        }
        }
    }
}

namespace rhi::vulkan
{
    VulkanShaderCompiler::VulkanShaderCompiler(Options options) : ShaderCompiler(options)
    {
        static bool glslangInitialized = false;
        if (!glslangInitialized)
        {
            glslang::InitializeProcess();
            glslangInitialized = true;
        }
    }

    VulkanShaderCompiler::~VulkanShaderCompiler()
    {
        glslang::FinalizeProcess();
    }

    ShaderData VulkanShaderCompiler::Compile(std::string_view path)
	{
        rhi::log::info("Compiling {}", path);
        Context ctx;
        ctx.m_path = path;

        const auto rawShaderStr = ReadShader(path);
        if (rawShaderStr.empty())
        {
            return {};
        }

        ctx.m_rawCodeStr = rawShaderStr;

        const auto processedShaderStr = PreprocessShader(ctx);
        if (processedShaderStr.empty())
        {
            return {};
        }

        ctx.m_processedCodeStr = processedShaderStr;

#ifdef SHADER_COMPILER_VERBOSE
        rhi::log::debug("Preprocessed shader code: {}. Path: {}", processedShaderStr, path);
#endif

        ctx.m_type = TypeByExtension(path);

        ShaderData data;
        data.m_type = ctx.m_type;

        auto blob = CompileShader(ctx);
        if (blob.empty())
        {
            rhi::log::error("Shader compilation failed: {}", path);
            return {};
        }
        data.m_compiledShader = std::move(blob);
        data.m_valid = true;

        rhi::log::info("Successfully compiled: {}", path);

		return data;
	}

    core::Blob VulkanShaderCompiler::CompileShader(const Context& ctx)
    {
        TBuiltInResource resource = InitResources();
        glslang_input_t input{};
        input.language = GLSLANG_SOURCE_GLSL;
        input.stage = RHITypeToGLSLangType(ctx.m_type);
        input.client = GLSLANG_CLIENT_VULKAN;
#ifdef R_APPLE
        input.client_version = GLSLANG_TARGET_VULKAN_1_0;
#else
        input.client_version = GLSLANG_TARGET_VULKAN_1_2;
#endif
        input.target_language = GLSLANG_TARGET_SPV;
#ifdef R_APPLE
        input.target_language_version = GLSLANG_TARGET_SPV_1_0;
#else
        input.target_language_version = GLSLANG_TARGET_SPV_1_3;
#endif
        input.code = ctx.m_processedCodeStr.data();
        input.default_version = 450;
        input.default_profile = GLSLANG_NO_PROFILE;
        input.force_default_version_and_profile = false;
        input.forward_compatible = false;
        input.messages = GLSLANG_MSG_DEFAULT_BIT;
        input.resource = reinterpret_cast<const glslang_resource_t*>(&resource);

        glslang_shader_t* shader = glslang_shader_create(&input);

        if (!glslang_shader_preprocess(shader, &input))
        {
            rhi::log::error("[glslang] Shader {} preprocessing failed.\n{}\n{}",
									ctx.m_path,
					                glslang_shader_get_info_log(shader),
					                glslang_shader_get_info_debug_log(shader));
            glslang_shader_delete(shader);
            return {};
        }

        if (!glslang_shader_parse(shader, &input))
        {
            rhi::log::error("[glslang] Shader {} parsing failed.\n{}\n{}",
									ctx.m_path,
					                glslang_shader_get_info_log(shader),
					                glslang_shader_get_info_debug_log(shader));
            glslang_shader_delete(shader);
            return {};
        }

        glslang_program_t* program = glslang_program_create();
        glslang_program_add_shader(program, shader);

        if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
        {
            rhi::log::error("[glslang] Shader {} linking failed.\n{}\n{}",
									ctx.m_path,
					                glslang_program_get_info_log(program),
					                glslang_program_get_info_debug_log(program));
            glslang_program_delete(program);
            glslang_shader_delete(shader);
            return {};
        }

        glslang_program_SPIRV_generate(program, RHITypeToGLSLangType(ctx.m_type));

        std::vector<uint32_t> spirvBinary(glslang_program_SPIRV_get_size(program));
        glslang_program_SPIRV_get(program, spirvBinary.data());

        spirv_cross::Compiler spirvCompiler(spirvBinary.data(), spirvBinary.size());
        spirv_cross::ShaderResources res = spirvCompiler.get_shader_resources();

        //TODO: Reflection

        core::Blob shaderBinary(spirvBinary.data(), sizeof(spirvBinary[0]) * spirvBinary.size());

        const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
        if (spirv_messages)
        {
#ifdef SHADER_COMPILER_VERBOSE 
            rhi::log::debug("[glslang] {} SPIRV messages: {}", ctx.m_path, spirv_messages);
#endif
        }

        glslang_program_delete(program);
        glslang_shader_delete(shader);

        return std::move(shaderBinary);
    }

    std::string VulkanShaderCompiler::ReadShader(std::string_view path) const
	{
        std::ifstream stream(std::string{ path });
        
        if (!stream.is_open())
        {
            rhi::log::error("Could not find shader file: {}", path);
            return "";
        }

        std::string line;
        std::stringstream ss;

        while (std::getline(stream, line))
        {
            ss << line << '\n';
        }

        return ss.str();
	}

	std::string VulkanShaderCompiler::PreprocessShader(const Context& ctx)
	{
        std::istringstream stream(std::string{ ctx.m_rawCodeStr });
        std::string line;
        std::stringstream processedSource;

        while (std::getline(stream, line)) 
        {
            if (line.find("#include") != std::string::npos) 
            {
                size_t start = line.find("\"");
                size_t end = line.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos) 
                {
                    const auto shaderDir = fs::path(ctx.m_path).parent_path().generic_u8string();
                    std::string includePath = fmt::format("{}/{}", shaderDir, line.substr(start + 1, end - start - 1));
                    std::string includedContent;

                    {
                        std::lock_guard l(m_includeCacheMutex);
                        if (const auto it = m_includeCache.find(includePath); it != m_includeCache.end())
                        {
                            includedContent = it->second;
                        }
                    }

                    if (includedContent.empty())
                    {
                        includedContent = ReadShader(includePath);
                        {
                            std::lock_guard l(m_includeCacheMutex);
                            m_includeCache[includePath] = includedContent;
                        }
                    }

                	processedSource << includedContent << "\n";
                }
            }
            else 
            {
                processedSource << line << "\n";
            }
        }

        return processedSource.str();
	}
}
