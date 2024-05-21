#include <Engine/Editor/Panel.hpp>
#include <imgui.h>

namespace engine::editor
{

void Panel::Draw()
{
	ImGui::Begin(m_name.c_str());

	m_isHovered = ImGui::IsWindowHovered();

	DrawPanel();

	ImGui::End();
}

} // engine::editor