#pragma once

#include <RHI/Texture.hpp>
#include <RHI/Buffer.hpp>
#include <vulkan/vulkan.h>

#pragma warning(push)
#pragma warning(disable : 4189)
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#pragma warning(pop)

namespace rhi::vulkan
{
	class RHI_API VulkanTexture : public Texture
	{
	public:
		VulkanTexture(const TextureDescriptor& desc, const std::shared_ptr<Sampler>& sampler, const void* data = nullptr);

		virtual ~VulkanTexture() override;

		void			ChangeImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);
		void			ChangeImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

		VkImageLayout	Layout() const { return m_layout; }
		VkImage			Image() const { return m_image; }
		VkImageView		ImageView(uint32_t idx) const { RHI_ASSERT(idx < m_imageViews.size());  return m_imageViews[idx]; }

	private:
		void ChangeImageLayout(VkCommandBuffer cmdBuffer,
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			Format format,
			int layers,
			int mipmaps,
			bool isDepth = false);

		// Immediate change, may cause deadlocks!
		void ChangeImageLayout(VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			Format format,
			int layers,
			int mipmaps,
			bool isDepth = false);

		std::shared_ptr<Buffer>		m_stagingBuffer;
		VkImage						m_image = nullptr;
		eastl::vector<VkImageView>	m_imageViews;
		VmaAllocation				m_allocation = nullptr;
		VkImageLayout				m_layout;
	};
}
