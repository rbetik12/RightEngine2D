#pragma once

#include <RHI/Texture.hpp>
#include <EASTL/vector.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace rhi
{

enum class AttachmentLoadOperation : uint8_t
{
    UNDEFINED = 0,
    LOAD,
    CLEAR
};

enum class AttachmentStoreOperation : uint8_t
{
    UNDEFINED = 0,
    STORE
};

struct ClearValue
{
    glm::vec4   m_color{ 0.0f, 0.0f, 0.0f, 1.0f };
    float       m_depth = 1.0f;
    uint32_t    m_stencil = 0;
};

struct AttachmentDescriptor
{
    std::shared_ptr<Texture>    m_texture;
    ClearValue                  m_clearValue;
    AttachmentLoadOperation     m_loadOperation = AttachmentLoadOperation::CLEAR;
    AttachmentStoreOperation    m_storeOperation = AttachmentStoreOperation::STORE;
};

struct RenderPassDescriptor
{
    glm::ivec2                              m_extent{ 0, 0 };
    eastl::vector<AttachmentDescriptor>     m_colorAttachments;
    AttachmentDescriptor                    m_depthStencilAttachment;
    std::string                             m_name;
};

}