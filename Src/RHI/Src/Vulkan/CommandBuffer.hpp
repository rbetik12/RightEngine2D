#pragma once

#include <RHI/Config.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{

class RHI_API CommandBuffer
{
public:
    CommandBuffer();

    ~CommandBuffer();

    using PayloadFn = std::function<void(VkCommandBuffer)>;

    inline void Push(PayloadFn&& fn) { fn(m_handle); }

    void Begin(bool oneTimeUsage = true);
    void End();
    void Reset();

    VkCommandBuffer Raw() { return m_handle; }

private:
    VkCommandBuffer m_handle;
};

}