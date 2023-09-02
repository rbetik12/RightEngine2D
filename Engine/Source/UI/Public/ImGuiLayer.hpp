#pragma once

#include "Layer.hpp"
#include "ImguiLayerImpl.hpp"
#include <imgui.h>

namespace RightEngine
{
    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer(const std::shared_ptr<GraphicsPipeline>& aPipeline);
        ~ImGuiLayer() override = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(float ts) override;

        virtual void OnEvent(Event& event) override;

        void Begin();
        void End(const std::shared_ptr<CommandBuffer>& cmd);

        static void Image(const std::shared_ptr<Texture>& texture,
                          const ImVec2& size,
                          const ImVec2& uv0 = ImVec2(0, 0),
                          const ImVec2& uv1 = ImVec2(1, 1));

        static void ImageButton(const std::shared_ptr<Texture>& texture,
            const ImVec2& size,
            const ImVec2& uv0 = ImVec2(0, 0),
            const ImVec2& uv1 = ImVec2(1, 1));

    private:
        void CreateImpl();

        static std::shared_ptr<ImguiLayerImpl> impl;
        std::shared_ptr<GraphicsPipeline> pipeline;
    };
}
