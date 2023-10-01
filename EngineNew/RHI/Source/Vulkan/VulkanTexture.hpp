#pragma once

#include <RHI/Texture.hpp>
#include <RHI/Buffer.hpp>
#include <vulkan/vulkan.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

namespace rhi::vulkan
{
	class RHI_API VulkanTexture : public Texture
	{
	public:
		VulkanTexture(const TextureDescriptor& desc, const eastl::vector<uint8_t>& data);

		virtual ~VulkanTexture() override;

	private:
		std::shared_ptr<Buffer>		m_stagingBuffer;
		VkImage						m_image;
		eastl::vector<VkImageView>	m_imageViews;
		VmaAllocation				m_allocation;
		VkImageLayout				m_layout;
	};
}
