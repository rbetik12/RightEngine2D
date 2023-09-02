#pragma once

#include <vulkan/vulkan.h>
#include "VulkanCommandBuffer.hpp"
#include "VulkanDevice.hpp"

namespace RightEngine
{
    class VulkanUtils
    {
    public:
        static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        static VkDeviceMemory AllocateMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags flags);
        static void BeginCommandBuffer(const std::shared_ptr<VulkanCommandBuffer>& cmd, bool oneTimeUsage);
        static void EndCommandBuffer(const std::shared_ptr<VulkanDevice>& device,
                                     const std::shared_ptr<VulkanCommandBuffer>& cmd);
        static void EndCommandBuffer(const std::shared_ptr<VulkanDevice>& device,
									 const std::shared_ptr<VulkanCommandBuffer>& cmd,
									 const VkSubmitInfo& info,
									 VkFence fence = VK_NULL_HANDLE);
        static void CopyBuffer(const std::shared_ptr<VulkanCommandBuffer>& cmd,
                               VkBuffer dst,
                               VkBuffer src,
                               size_t size);

    private:
        static std::mutex s_mutex;
    };
}