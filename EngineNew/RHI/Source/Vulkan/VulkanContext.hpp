#pragma once

#include <RHI/IContext.hpp>

namespace rhi::vulkan
{

	class RHI_API VulkanContext : public IContext
	{
	public:
		VulkanContext(VulkanInitContext&& ctx);

		virtual ~VulkanContext() override;

		inline VkInstance	Instance() const { return m_instance; }
		inline VkSurfaceKHR Surface() const { return m_surface; }

		const eastl::vector<const char*>& ValidationLayers() const;

	private:
		uint32_t		m_extensionAmount{ 0 };
		VkSurfaceKHR	m_surface;
		VkInstance		m_instance;
	};

}
