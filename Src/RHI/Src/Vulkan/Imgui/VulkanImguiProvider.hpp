#pragma once

#include <RHI/IImguiProvider.hpp>
#include <vulkan/vulkan.h>

namespace rhi::vulkan::imgui
{

class RHI_API VulkanImguiProvider : public rhi::imgui::IImguiProvider
{
public:
    VulkanImguiProvider();
    virtual ~VulkanImguiProvider() override;

    virtual void Begin() override;
    virtual void End() override;

private:
    void CreateDescriptorPool();

    VkDescriptorPool m_descriptorPool = nullptr;
};

} // rhi::vulkan::imgui