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
    virtual void Image(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) override;

private:
    void CreateDescriptorPool();

    VkDescriptorSet GetDescriptorSet(const std::shared_ptr<Texture>& texture);

    VkDescriptorPool                                    m_descriptorPool = nullptr;
    eastl::unordered_map<VkImageView, VkDescriptorSet>  m_imageViewToDescSet;
};

} // rhi::vulkan::imgui