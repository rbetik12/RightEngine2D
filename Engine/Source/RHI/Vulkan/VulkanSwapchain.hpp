#pragma once

#include "VulkanDevice.hpp"
#include "Types.hpp"
#include <glm/vec2.hpp>

namespace RightEngine
{
    struct SwapchainDescriptor
    {
        Format format;
        PresentMode presentMode;
        glm::ivec2 extent;
    };

    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(const std::shared_ptr<Device>& device,
                        const std::shared_ptr<Surface>& surface,
                        const SwapchainDescriptor& descriptor);

        virtual ~VulkanSwapchain();

        const std::vector<VkImage>& GetImages() const
        { return swapChainImages; }

        const std::vector<VkImageView>& GetImageViews() const
        { return swapChainImageViews; }

        const SwapchainDescriptor& GetDescriptor() const
        { return descriptor; }

        const VkSurfaceFormatKHR GetImageFormat() const
        { return imageFormat; }

        const VkSwapchainKHR GetSwapchain() const
        { return swapChain; }

    private:
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        SwapchainSupportDetails details;
        SwapchainDescriptor descriptor;
        VkSurfaceFormatKHR imageFormat;

        void Init(const std::shared_ptr<VulkanDevice>& device,
                  const std::shared_ptr<VulkanSurface>& surface,
                  const SwapchainDescriptor& descriptor);
    };
}
