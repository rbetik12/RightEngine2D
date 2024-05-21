#pragma once

#include <Engine/Editor/Panel.hpp>

namespace engine::editor
{

class ENGINE_API EntityTreePanel final : public Panel
{
public:
	EntityTreePanel() : Panel("Entities")
	{}

private:
	virtual void DrawPanel() override;
};

} // engine::editor