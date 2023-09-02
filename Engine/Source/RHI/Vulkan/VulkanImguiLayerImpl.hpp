#pragma once

#include "ImGuiLayerImpl.hpp"
#include <vulkan/vulkan.h>

namespace RightEngine
{
    class VulkanImguiLayerImpl : public ImguiLayerImpl
    {
    public:
        virtual ~VulkanImguiLayerImpl() override = default;

        virtual void OnAttach(const std::shared_ptr<GraphicsPipeline>& pipeline) override;
        virtual void OnDetach() override;

        virtual void OnUpdate(float deltaTime) override;

        virtual void OnEvent(Event& event) override;

        virtual void Begin() override;
        virtual void End(const std::shared_ptr<CommandBuffer>& cmd) override;

        virtual void Image(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) override;
        virtual void ImageButton(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) override;

    private:
        VkDescriptorSet GetDescriptorSet(const std::shared_ptr<Texture>& texture);

        VkRenderPass renderPass;
        VkDescriptorPool imguiPool;
        std::unordered_map<VkImageView, VkDescriptorSet> imageViewToDescriptorSet;
        std::string iniFilePath;
    };
}