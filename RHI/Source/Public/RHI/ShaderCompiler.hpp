#pragma once

#include <RHI/Config.hpp>
#include <RHI/ShaderDescriptor.hpp>
#include <Core/Blob.hpp>

namespace rhi
{

    struct CompiledShaderData
    {
        ShaderReflection    m_reflection;
        core::Blob          m_compiledShader;
        ShaderType          m_type = ShaderType::NONE;
        bool                m_valid = false;
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

        // Path must be absolute
        virtual CompiledShaderData Compile(std::string_view path) = 0;

        // Accepts shader code as a text buffer
        virtual CompiledShaderData CompileText(std::string_view text, ShaderType type, std::string_view name = "") = 0;

    protected:
        Options m_options;
    };

}