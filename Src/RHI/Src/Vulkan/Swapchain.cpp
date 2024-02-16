#include "Swapchain.hpp"
#include "VulkanDevice.hpp"
#include <vk-tools/VulkanTools.h>

#undef max

namespace rhi::vulkan
{

namespace
{

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const eastl::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const SwapchainDescriptor& descriptor)
{
    if (descriptor.m_extent.x > capabilities.maxImageExtent.width
        || descriptor.m_extent.x < capabilities.minImageExtent.width
        || descriptor.m_extent.y > capabilities.maxImageExtent.height
        || descriptor.m_extent.y < capabilities.minImageExtent.height)
    {
        rhi::log::warning("Swapchain has size that is out of bound of surface size. Swapchain: {}x{} Surface bounds: {}-{}x{}-{}",
            descriptor.m_extent.x,
            descriptor.m_extent.y,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);
    }

    VkExtent2D actualExtent =
    {
    static_cast<uint32_t>(descriptor.m_extent.x),
    static_cast<uint32_t>(descriptor.m_extent.y)
    };

    actualExtent.width = glm::clamp(actualExtent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);
    actualExtent.height = glm::clamp(actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);

    return actualExtent;
}

} // namespace unnamed

Swapchain::Swapchain(const SwapchainDescriptor& desc) : m_descriptor(desc)
{
    RHI_ASSERT(desc.m_surface);
    auto& device = *VulkanDevice::s_ctx.m_instance;
    SwapchainSupportDetails details = VulkanDevice::s_ctx.m_instance->GetSwapchainSupportDetails();

    const auto imageFormat = ChooseSwapSurfaceFormat(details.m_formats);
    VkPresentModeKHR presentMode = desc.m_presentMode;
    VkExtent2D extent = ChooseSwapExtent(details.m_capabilities, m_descriptor);

    uint32_t imageCount = details.m_capabilities.minImageCount + 1;
    if (details.m_capabilities.maxImageCount > 0 && imageCount > details.m_capabilities.maxImageCount)
    {
        imageCount = details.m_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = desc.m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = imageFormat.format;
    createInfo.imageColorSpace = imageFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    QueueFamilyIndices indices = device.FindQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = details.m_capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    const auto error = vkCreateSwapchainKHR(VulkanDevice::s_ctx.m_device, &createInfo, nullptr, &m_swapchain);
    RHI_ASSERT(error == VK_SUCCESS);

    vkGetSwapchainImagesKHR(VulkanDevice::s_ctx.m_device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(VulkanDevice::s_ctx.m_device, m_swapchain, &imageCount, m_images.data());

    m_imageViews.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = imageFormat.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        RHI_ASSERT(vkCreateImageView(VulkanDevice::s_ctx.m_device, &imageViewCreateInfo, nullptr, &m_imageViews[i]) == VK_SUCCESS);
    }

    CommandBuffer cmdBuffer;
    cmdBuffer.Begin();

    VkImageSubresourceRange srcSubRange{};
    srcSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcSubRange.baseMipLevel = 0;
    srcSubRange.levelCount = 1;
    srcSubRange.layerCount = 1;

    for (auto& image : m_images)
    {
        vks::tools::setImageLayout(cmdBuffer.Raw(),
            image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            srcSubRange);
    }
    cmdBuffer.End();

    VulkanDevice::s_ctx.m_instance->Execute(cmdBuffer)->Wait();
}

Swapchain::~Swapchain()
{
    for (auto imageView : m_imageViews)
    {
        vkDestroyImageView(VulkanDevice::s_ctx.m_device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(VulkanDevice::s_ctx.m_device, m_swapchain, nullptr);
}

uint32_t Swapchain::AcquireNextImage(VkDevice device, VkSemaphore presentSemaphore)
{
    uint32_t imageIndex;
    RHI_ASSERT(vkAcquireNextImageKHR(device, m_swapchain, UINT64_MAX, presentSemaphore, (VkFence)nullptr, &imageIndex) == VK_SUCCESS);
    return imageIndex;
}

} //namespace rhi::vulkan
