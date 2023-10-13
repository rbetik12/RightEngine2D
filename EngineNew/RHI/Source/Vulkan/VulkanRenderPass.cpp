#include "VulkanRenderPass.hpp"
#include "VulkanDevice.hpp"
#include "VulkanTexture.hpp"
#include "VulkanHelpers.hpp"
#include <vulkan/vulkan.h>


namespace rhi::vulkan
{

VulkanRenderPass::VulkanRenderPass(const RenderPassDescriptor& descriptor) : RenderPass(descriptor)
{
    eastl::vector<VkAttachmentDescription> colorAttachments;
    for (const auto& attachment : m_descriptor.m_colorAttachments)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = helpers::Format(attachment.m_texture->Descriptor().m_format);
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = helpers::LoadOperation(attachment.m_loadOperation);
        colorAttachment.storeOp = helpers::StoreOperation(attachment.m_storeOperation);
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        if (attachment.m_loadOperation == AttachmentLoadOperation::LOAD)
        {
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        colorAttachments.push_back(colorAttachment);
        m_initialLayoutColor.push_back(colorAttachment.initialLayout);
        m_finalLayoutColor.push_back(colorAttachment.finalLayout);
    }

    VkAttachmentDescription depthAttachment{};
    if (m_descriptor.m_depthStencilAttachment.m_texture)
    {
        depthAttachment.format = helpers::Format(m_descriptor.m_depthStencilAttachment.m_texture->Descriptor().m_format);
    }
    else
    {
        depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = helpers::LoadOperation(m_descriptor.m_depthStencilAttachment.m_loadOperation);
    depthAttachment.storeOp = helpers::StoreOperation(m_descriptor.m_depthStencilAttachment.m_storeOperation);
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    if (depthAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
    {
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else
    {
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_initialLayoutDepth = depthAttachment.initialLayout;
    m_finalLayoutDepth = depthAttachment.finalLayout;

    eastl::vector<VkAttachmentReference> colorAttachmentRefs;

    for (int i = 0; i < colorAttachments.size(); i++)
    {
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = i;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachmentRefs.push_back(colorAttachmentRef);
    }

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = static_cast<uint32_t>(colorAttachmentRefs.size());
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();
    const bool hasDepthAttachment = m_descriptor.m_depthStencilAttachment.m_texture != nullptr;
    if (hasDepthAttachment)
    {
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    }

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (hasDepthAttachment)
    {
        colorAttachments.push_back(depthAttachment);
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(colorAttachments.size());
    renderPassInfo.pAttachments = colorAttachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    RHI_ASSERT(vkCreateRenderPass(VulkanDevice::s_ctx.m_device, &renderPassInfo, nullptr, &m_pass) == VK_SUCCESS);

    CreateFramebuffer();
}

VulkanRenderPass::~VulkanRenderPass()
{
    vkDestroyFramebuffer(VulkanDevice::s_ctx.m_device, m_framebuffer, nullptr);
    vkDestroyRenderPass(VulkanDevice::s_ctx.m_device, m_pass, nullptr);
}

void VulkanRenderPass::CreateFramebuffer()
{
    eastl::vector<VkImageView> attachments;

    for (const auto& attachment : m_descriptor.m_colorAttachments)
    {
        const auto& texDescriptor = attachment.m_texture->Descriptor();
        RHI_ASSERT(texDescriptor.m_width == m_descriptor.m_extent.x
				&& texDescriptor.m_height == m_descriptor.m_extent.y);
        const auto vkTexture = std::static_pointer_cast<VulkanTexture>(attachment.m_texture);
        attachments.push_back(vkTexture->ImageView(0));
    }

    if (m_descriptor.m_depthStencilAttachment.m_texture)
    {
        attachments.push_back(std::static_pointer_cast<VulkanTexture>(m_descriptor.m_depthStencilAttachment.m_texture)->ImageView(0));
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_descriptor.m_extent.x;
    framebufferInfo.height = m_descriptor.m_extent.y;
    framebufferInfo.layers = 1;

    RHI_ASSERT(vkCreateFramebuffer(VulkanDevice::s_ctx.m_device, &framebufferInfo, nullptr, &m_framebuffer) != VK_SUCCESS);
}

}
