#pragma once

#include <Core/Blob.hpp>
#include <RHI/VertexBufferLayout.hpp>
#include <RHI/BufferDescriptor.hpp>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>
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
            BufferType m_type;
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
        std::string         m_name;
        std::string         m_path;
        ShaderType          m_type = ShaderType::NONE;
        ShaderReflection    m_reflection;
        core::Blob          m_blob;
    };

}
