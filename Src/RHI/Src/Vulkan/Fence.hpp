#pragma once

#include <RHI/Config.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan
{

class RHI_API Fence
{
public:
    Fence(bool signaled = false);
    ~Fence();

    void Wait();
    void Reset();
    bool IsSignaled();

    VkFence* Raw() { return &m_handle; }

private:
    VkFence m_handle;
};

}