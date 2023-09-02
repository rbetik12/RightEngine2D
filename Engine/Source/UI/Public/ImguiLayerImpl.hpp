#pragma once

#include "Event.hpp"
#include "CommandBuffer.hpp"
#include "GraphicsPipeline.hpp"
#include <imgui.h>

namespace RightEngine
{
    class ImguiLayerImpl
    {
    public:
        ImguiLayerImpl() = default;
        virtual ~ImguiLayerImpl() = default;

        virtual void OnAttach(const std::shared_ptr<GraphicsPipeline>& pipeline) = 0;
        virtual void OnDetach() = 0;

        virtual void OnUpdate(float deltaTime) = 0;

        virtual void OnEvent(Event& event) = 0;

        virtual void Begin() = 0;
        virtual void End(const std::shared_ptr<CommandBuffer>& cmd) = 0;

        virtual void Image(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) = 0;
        virtual void ImageButton(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) = 0;
    };
}