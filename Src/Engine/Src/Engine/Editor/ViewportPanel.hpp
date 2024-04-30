#pragma once

#include <Engine/Editor/Panel.hpp>
#include <imgui.h>

namespace engine::editor
{

class ENGINE_API ViewportPanel final : public Panel
{
public:
	ViewportPanel() : Panel("Viewport")
	{}

	glm::ivec2 Size() const
	{
		return { m_viewportSize.x, m_viewportSize.y };
	}

private:
	virtual void DrawPanel() override;

	ImVec2 m_viewportSize = ImVec2(1, 1);
};

} // engine::editor