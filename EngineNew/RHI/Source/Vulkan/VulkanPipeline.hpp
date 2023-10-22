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

private:
	VkPipelineLayout	m_layout;
	VkPipeline			m_pipeline;
};

} // namespace rhi::vulkan