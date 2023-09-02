#include "ImGuiLayer.hpp"

using namespace RightEngine;

ImGuiLayer::ImGuiLayer(const std::shared_ptr<GraphicsPipeline>& aPipeline): pipeline(aPipeline), Layer("ImGuiLayer")
{
    CreateImpl();
}

void ImGuiLayer::OnAttach()
{
    impl->OnAttach(pipeline);
}

void ImGuiLayer::OnDetach()
{
    impl->OnDetach();
}

void ImGuiLayer::OnUpdate(float ts)
{
    impl->OnUpdate(ts);
}

void ImGuiLayer::OnEvent(Event& event)
{
    impl->OnEvent(event);
}

void ImGuiLayer::Begin()
{
    impl->Begin();
}

void ImGuiLayer::End(const std::shared_ptr<CommandBuffer>& cmd)
{
    impl->End(cmd);
}

void ImGuiLayer::Image(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
{
    impl->Image(texture, size, uv0, uv1);
}

void ImGuiLayer::ImageButton(const std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
{
    impl->ImageButton(texture, size, uv0, uv1);
}
