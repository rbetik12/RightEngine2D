#pragma once

#include <RHI/Config.hpp>
#include <RHI/BufferDescriptor.hpp>
#include <RHI/VertexBufferLayout.hpp>
#include <Core/Hash.hpp>
#include <Core/Blob.hpp>
#include <EASTL/unordered_map.h>
#include <EASTL/vector.h>
#include <string>

namespace rhi
{
    enum class ShaderType : uint8_t
    {
        NONE = 0,
        VERTEX,
        FRAGMENT
    };
    typedef ShaderType ShaderStage;

    struct ShaderReflection
    {
        struct BufferInfo
        {
            rhi::BufferType m_type;
            std::string     m_name;
        };

        struct TextureInfo
        {
            uint8_t     m_slot;
            std::string m_name;
        };

        using BufferMap = eastl::unordered_map<uint8_t, BufferInfo>;
        using TextureList = eastl::vector<TextureInfo>;

        BufferMap           m_bufferMap;
    	TextureList         m_textures;
        VertexBufferLayout  m_inputLayout;
    };

    struct ShaderDescriptor
    {
        std::string name;
        std::string path;
        ShaderType type = ShaderType::NONE;
    };

    struct ShaderProgramDescriptor
    {
        eastl::vector<ShaderDescriptor> shaders;
        VertexBufferLayout layout;
        ShaderReflection reflection;
    };

    struct ShaderData
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
        //TODO: Implement options: switch Vulkan API version, shader code version, etc
        struct Options
        {};

        ShaderCompiler(Options options) : m_options(options)
        {}

        virtual ~ShaderCompiler() = default;

        //Path must be absolute
        virtual ShaderData Compile(std::string_view path) = 0;

    protected:
        Options m_options;
    };
}