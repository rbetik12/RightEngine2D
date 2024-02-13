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

	using AttachmentInfo = VkRenderingAttachmentInfoKHR;
	using AttachmentInfoList = eastl::vector<VkRenderingAttachmentInfoKHR>;

	const AttachmentInfoList&	ColorAttachments() const { return m_colorAttachmentInfos; }
	const AttachmentInfo&		DepthAttachment() const { return m_depthAttachmentInfo; }

private:
	AttachmentInfoList				m_colorAttachmentInfos;
	AttachmentInfo					m_depthAttachmentInfo;
};

}