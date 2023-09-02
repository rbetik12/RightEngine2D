#include "VulkanSampler.hpp"
#include "VulkanDevice.hpp"
#include "VulkanConverters.hpp"

using namespace RightEngine;

VulkanSampler::VulkanSampler(const std::shared_ptr<Device>& device,
                             const SamplerDescriptor& descriptor) : Sampler(device, descriptor)
{
    const auto vkDevice = std::static_pointer_cast<VulkanDevice>(device);
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(vkDevice->GetPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VulkanConverters::AddressMode(descriptor.addressModeU);
    samplerInfo.addressModeV = VulkanConverters::AddressMode(descriptor.addressModeV);
    samplerInfo.addressModeW = VulkanConverters::AddressMode(descriptor.addressModeW);
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = descriptor.minLod;
    samplerInfo.maxLod = descriptor.maxLod;

    if (vkCreateSampler(vkDevice->GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        R_CORE_ASSERT(false, "failed to create texture sampler!");
    }
}

VulkanSampler::~VulkanSampler()
{
    vkDestroySampler(VK_DEVICE()->GetDevice(), sampler, nullptr);
}
