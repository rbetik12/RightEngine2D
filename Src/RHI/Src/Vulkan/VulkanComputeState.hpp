#pragma once

#include <Rhi/ComputeState.hpp>
#include <Vulkan/CommandBuffer.hpp>

namespace rhi::vulkan
{

struct RHI_API VulkanComputeState : public ComputeState
{
    VulkanComputeState()
    {
        m_cmdBuffer.Begin();
    }

    CommandBuffer m_cmdBuffer;
};

} // rhi::vulkan