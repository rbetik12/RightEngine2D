#pragma once

#include <RHI/Pipeline.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{

class RHI_API VulkanPipeline : public Pipeline
{
public:
	VulkanPipeline(const PipelineDescriptor& descriptor);

	virtual ~VulkanPipeline() override;

	VkPipeline			GetPipeline() const { return m_pipeline; }
	VkPipelineLayout	Layout() const { return m_layout; }

private:
	VkPipelineLayout	m_layout;
	VkPipeline			m_pipeline;
};

} // namespace rhi::vulkan