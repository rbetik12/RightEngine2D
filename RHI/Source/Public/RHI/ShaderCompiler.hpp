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

    struct RHI_API BufferRef
    {
        int slot;
        ShaderStage stage;

        bool operator==(const BufferRef& other) const
        {
            return slot == other.slot && stage == other.stage;
        }
    };

    struct RHI_API BufferRefHash
    {
        size_t operator()(const BufferRef& p) const
        {
            size_t h = 0x1231;
            core::hash::CombineHash(h, p.slot);
            core::hash::CombineHash(h, p.stage);
            return h;
        }
    };

    struct ShaderReflection
    {
        using BufferMap = eastl::unordered_map<BufferRef, BufferType, BufferRefHash>;
        BufferMap buffers;
        eastl::vector<int> textures;
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