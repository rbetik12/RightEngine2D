#pragma once

#include <RHI/IImguiProvider.hpp>
#include <vulkan/vulkan.h>
#include <shared_mutex>

namespace rhi::vulkan::imgui
{

class RHI_API VulkanImguiProvider : public rhi::imgui::IImguiProvider
{
public:
    VulkanImguiProvider();
    virtual ~VulkanImguiProvider() override;

    virtual void            Begin() override;
    virtual void            End() override;
    virtual ImTextureID     Image(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) override;
    virtual void            RemoveImage(const std::shared_ptr<Texture>& texture) override;

private:
    void CreateDescriptorPool();

    VkDescriptorSet GetDescriptorSet(const std::shared_ptr<Texture>& texture);

    VkDescriptorPool                                    m_descriptorPool = nullptr;
    eastl::unordered_map<VkImageView, VkDescriptorSet>  m_imageViewToDescSet;
    std::shared_mutex                                   m_imageMapMutex;
};

} // rhi::vulkan::imgui