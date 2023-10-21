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
    FRAGMENT
};

inline std::string_view ShaderStageToString(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::NONE: return "None";
	case ShaderStage::VERTEX: return "Vertex";
	case ShaderStage::FRAGMENT: return "Fragment";
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
        BufferType      m_type;
        ShaderStage     m_stage = ShaderStage::NONE;
        std::string     m_name;
    };

    struct TextureInfo
    {
        uint8_t     m_slot;
        std::string m_name;

        inline bool operator<(const TextureInfo& other) const
        {
            return m_slot < other.m_slot;
        }
    };

    using BufferMap = eastl::unordered_map<uint8_t, BufferInfo>;
    using TextureList = eastl::vector_set<TextureInfo>;

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

    using BlobMap = eastl::unordered_map<ShaderStage, core::Blob>;

	BlobMap             m_blobByStage;
};

}
