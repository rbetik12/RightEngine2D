#pragma once

#include <Engine/Editor/Panel.hpp>

namespace engine::editor
{

class ENGINE_API ComponentPanel final : public Panel
{
public:
	ComponentPanel() : Panel("Components")
	{}

private:
	virtual void DrawPanel() override;
};

} // engine::editor