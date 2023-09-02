#pragma once

#include "VertexBufferLayout.hpp"
#include "Utils.hpp"
#include "BufferDescriptor.hpp"
#include <Core/Hash.hpp>
#include <string>

namespace RightEngine
{
    enum class ShaderType
    {
        NONE = 0,
        VERTEX,
        FRAGMENT
    };
    typedef ShaderType ShaderStage;

    constexpr const int C_CONSTANT_BUFFER_SLOT = -1;

    struct BufferRef
    {
        int slot;
        ShaderStage stage;

        bool operator==(const BufferRef& other) const
        {
            return slot == other.slot && stage == other.stage;
        }
    };

    struct BufferRefHash
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
        using BufferMap = std::unordered_map<BufferRef, BufferType, BufferRefHash>;
        BufferMap buffers;
        std::vector<int> textures;
    };

    struct ShaderDescriptor
    {
        std::string name;
        std::string path;
        ShaderType type = ShaderType::NONE;
    };

    struct ShaderProgramDescriptor
    {
        std::vector<ShaderDescriptor> shaders;
        VertexBufferLayout layout;
        ShaderReflection reflection;
    };
}