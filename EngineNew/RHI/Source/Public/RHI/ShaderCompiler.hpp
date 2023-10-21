#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderDescriptor.hpp>
#include <Core/Blob.hpp>

namespace rhi
{

    struct CompiledShaderData
    {
        ShaderReflection                                m_reflection;
        eastl::unordered_map<ShaderStage, core::Blob>	m_stageBlob;
        bool                                            m_valid = false;
    };

    // API assumes that there will be only one instance of shader compiler in application, please follow that rule!
    class RHI_API ShaderCompiler
    {
    public:
        // TODO: Implement options: switch Vulkan API version, shader code version, etc
        struct Options
        {};

        ShaderCompiler(Options options) : m_options(options)
        {}

        virtual ~ShaderCompiler() = default;

        // TODO: Implement compiling from already giving shader text
        // Path must be absolute
        virtual CompiledShaderData Compile(std::string_view path, ShaderType type = ShaderType::FX) = 0;

    protected:
        Options m_options;
    };

}