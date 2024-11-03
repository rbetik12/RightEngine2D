#pragma once

#include <Core/Blob.hpp>
#include <RHI/VertexBufferLayout.hpp>
#include <RHI/BufferDescriptor.hpp>
#include <EASTL/vector_set.h>
#include <EASTL/unordered_map.h>
#include <string>

namespace rhi
{

enum class ShaderType : uint8_t
{
    NONE = 0,
    FX,
    COMPUTE
};

enum class ShaderStage : uint8_t
{
    NONE = 0,
    VERTEX,
    FRAGMENT,
    COMPUTE
};

inline std::string_view ShaderStageToString(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::NONE: return "None";
    case ShaderStage::VERTEX: return "Vertex";
    case ShaderStage::FRAGMENT: return "Fragment";
    case ShaderStage::COMPUTE: return "Compute";
    default:
        {
            RHI_ASSERT(false);
            return "";
        }
    }
}

struct ShaderReflection
{
    struct BufferInfo
    {
        std::string     m_name;
        uint32_t        m_size = 0;
        BufferType      m_type;
        ShaderStage     m_stage = ShaderStage::NONE;
    };

    struct TextureInfo
    {
        std::string m_name;
        uint8_t     m_slot;
        bool        m_isCubemap = false;

        inline bool operator<(const TextureInfo& other) const
        {
            return m_slot < other.m_slot;
        }
    };

    using BufferMap = eastl::unordered_map<uint8_t, BufferInfo>;
    using TextureList = eastl::vector_set<TextureInfo>;

    BufferMap           m_bufferMap;
    BufferMap           m_storageBufferMap;
    // Only in vertex shader now
    BufferInfo          m_pushConstant;
    TextureList         m_textures;
    TextureList         m_storageImages;
    VertexBufferLayout  m_inputLayout;
    // Only in fragment shader now
    uint8_t             m_outputAmount;
};

struct ShaderDescriptor
{
    std::string         m_name;
    std::string         m_path;
    ShaderType          m_type = ShaderType::NONE;
    ShaderReflection    m_reflection;

    using BlobMap = eastl::unordered_map<ShaderStage, core::Blob>;

    BlobMap             m_blobByStage;
};

}
