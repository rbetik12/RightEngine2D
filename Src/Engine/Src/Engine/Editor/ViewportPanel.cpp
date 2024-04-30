#include <Engine/Editor/ViewportPanel.hpp>
#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Service/Render/RenderService.hpp>

namespace engine::editor
{

void ViewportPanel::DrawPanel()
{
    auto& rs = Instance().Service<RenderService>();
    auto& is = Instance().Service<ImguiService>();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (!core::math::almostEqual(m_viewportSize.x, viewportSize.x) || !core::math::almostEqual(m_viewportSize.y, viewportSize.y))
    {
        m_viewportSize = { viewportSize.x, viewportSize.y };
        rs.OnResize(static_cast<uint32_t>(m_viewportSize.x), static_cast<uint32_t>(m_viewportSize.y));
    }

    is.Image(rs.BasicPass()->Descriptor().m_colorAttachments[0].m_texture, m_viewportSize);
}

} // engine::editor
