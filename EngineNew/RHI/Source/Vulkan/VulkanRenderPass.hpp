#pragma once

#include <RHI/RenderPass.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{

class RHI_API VulkanRenderPass : public RenderPass
{
public:
	VulkanRenderPass(const RenderPassDescriptor& descriptor);

	virtual ~VulkanRenderPass() override;

	VkImageLayout	InitialLayoutColor(uint32_t idx) const { RHI_ASSERT(idx < m_descriptor.m_colorAttachments.size()); return m_initialLayoutColor[idx]; }
	VkImageLayout	FinalLayoutColor(uint32_t idx) const { RHI_ASSERT(idx < m_descriptor.m_colorAttachments.size()); return m_finalLayoutColor[idx]; }

	VkImageLayout	InitialLayoutDepth() const { return m_initialLayoutDepth; }
	VkImageLayout	FinalLayoutDepth() const { return m_finalLayoutDepth; }

	bool			hasDepth() const { return m_descriptor.m_depthStencilAttachment.m_texture != nullptr; }

private:
	void CreateFramebuffer();

	eastl::vector<VkImageLayout>	m_initialLayoutColor;
	eastl::vector<VkImageLayout>	m_finalLayoutColor;
	VkImageLayout					m_initialLayoutDepth = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout					m_finalLayoutDepth = VK_IMAGE_LAYOUT_UNDEFINED;
	VkRenderPass					m_pass = nullptr;
	VkFramebuffer					m_framebuffer = nullptr;
};

}