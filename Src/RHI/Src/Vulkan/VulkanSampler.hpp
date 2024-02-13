#pragma once

#include <RHI/Sampler.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{
	class RHI_API VulkanSampler : public Sampler
	{
	public:
		VulkanSampler(const SamplerDescriptor& descriptor);

		virtual ~VulkanSampler() override;

		VkSampler Raw() const { return m_sampler; }

	private:
		VkSampler m_sampler;
	};
}
