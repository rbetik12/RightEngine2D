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

		void			ChangeImageLayout(VkImage image,
							VkImageLayout oldLayout,
							VkImageLayout newLayout,
							Format format,
							int layers,
							int mipmaps,
							bool isDepth = false);

		VkImage			Image() const { return m_image; }
		VkImageView		ImageView(uint32_t idx) const { RHI_ASSERT(idx < m_imageViews.size());  return m_imageViews[idx]; }

	private:
		std::shared_ptr<Buffer>		m_stagingBuffer;
		VkImage						m_image;
		eastl::vector<VkImageView>	m_imageViews;
		VmaAllocation				m_allocation;
		VkImageLayout				m_layout;
	};
}
