#include <Engine/Editor/Panel.hpp>
#include <imgui.h>

namespace engine::editor
{

void Panel::Draw()
{
	ImGui::Begin(m_name.c_str());
	DrawPanel();
	ImGui::End();
}

} // engine::editor