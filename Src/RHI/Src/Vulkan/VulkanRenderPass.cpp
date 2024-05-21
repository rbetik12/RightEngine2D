#include "VulkanRenderPass.hpp"
#include "VulkanDevice.hpp"
#include "VulkanTexture.hpp"
#include "VulkanHelpers.hpp"
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{

VulkanRenderPass::VulkanRenderPass(const RenderPassDescriptor& descriptor) : RenderPass(descriptor)
{
    // Color
    for (const auto& attachment : descriptor.m_colorAttachments)
    {
        const glm::vec4 color = attachment.m_clearValue.m_color;
        VkClearValue clearValue{};
        clearValue.color = { color.r, color.g, color.b, color.a };

        VkRenderingAttachmentInfoKHR colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        colorAttachment.loadOp = helpers::LoadOperation(attachment.m_loadOperation);
        colorAttachment.storeOp = helpers::StoreOperation(attachment.m_storeOperation);
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.clearValue = clearValue;
        colorAttachment.imageView = std::static_pointer_cast<VulkanTexture>(attachment.m_texture)->ImageView(0);

        m_colorAttachmentInfos.push_back(colorAttachment);
    }

    if (!HasDepth())
    {
        return;
    }

    // Depth
    const glm::vec4 color = descriptor.m_depthStencilAttachment.m_clearValue.m_color;
    VkClearValue clearValue{};
    clearValue.color = { color.r, color.g, color.b, color.a };
    clearValue.depthStencil.depth = descriptor.m_depthStencilAttachment.m_clearValue.m_depth;
    clearValue.depthStencil.stencil = descriptor.m_depthStencilAttachment.m_clearValue.m_stencil;

    std::memset(&m_depthAttachmentInfo, 0, sizeof(m_depthAttachmentInfo));
    m_depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    m_depthAttachmentInfo.clearValue = clearValue;
    m_depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    m_depthAttachmentInfo.imageView = std::static_pointer_cast<VulkanTexture>(descriptor.m_depthStencilAttachment.m_texture)->ImageView(0);
    m_depthAttachmentInfo.loadOp = helpers::LoadOperation(descriptor.m_depthStencilAttachment.m_loadOperation);
    m_depthAttachmentInfo.storeOp = helpers::StoreOperation(descriptor.m_depthStencilAttachment.m_storeOperation);
}

VulkanRenderPass::~VulkanRenderPass()
{
    
}

}
