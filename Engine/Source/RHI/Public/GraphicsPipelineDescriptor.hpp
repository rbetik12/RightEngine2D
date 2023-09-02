#pragma once

#include <glm/vec2.hpp>
#include <memory>
#include <unordered_map>

namespace RightEngine
{
    class Shader;
    class Buffer;
    class Texture;

    enum class CompareOp
    {
        LESS = 0,
        LESS_OR_EQUAL,
        GREATER
    };

    enum class CullMode
    {
        BACK = 0,
        FRONT
    };

    struct GraphicsPipelineDescriptor
    {
        std::shared_ptr<Shader> shader;
        CompareOp depthCompareOp{ CompareOp::LESS };
        CullMode cullMode{ CullMode::BACK };
    };

    enum class AttachmentLoadOperation
    {
        UNDEFINED = 0,
        LOAD,
        CLEAR
    };

    enum class AttachmentStoreOperation
    {
        UNDEFINED = 0,
        STORE
    };

    struct ClearValue
    {
        glm::vec4 color{0.0f, 0.0f, 0.0f, 1.0f};
        float depth = 1.0f;
        uint32_t stencil = 0;
    };

    struct AttachmentDescriptor
    {
        std::shared_ptr<Texture> texture;
        ClearValue clearValue;
        AttachmentLoadOperation loadOperation = AttachmentLoadOperation::CLEAR;
        AttachmentStoreOperation storeOperation = AttachmentStoreOperation::STORE;
    };

    struct RenderPassDescriptor
    {
        glm::ivec2 extent{0, 0};
        bool offscreen{false};
        std::vector<AttachmentDescriptor> colorAttachments;
        AttachmentDescriptor depthStencilAttachment;
        std::string name;
    };
}
