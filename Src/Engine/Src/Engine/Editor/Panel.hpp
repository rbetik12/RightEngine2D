#pragma once

#include <Engine/Config.hpp>

namespace engine::editor
{

class ENGINE_API Panel
{
public:
	Panel(std::string_view name) : m_name(name)
	{}

	virtual ~Panel() = default;

	virtual void Draw();

protected:

	virtual void DrawPanel() = 0;

	std::string m_name;
};

} // engine::editor