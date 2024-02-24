#pragma once

#include <Engine/Config.hpp>
#include <Engine/Assert.hpp>

namespace engine::ecs
{

class ENGINE_API Component
{
public:
	enum class Type : uint8_t
	{
		ENGINE = 0,
		PLUGIN
	};

	struct MetaInfo
	{
		Type m_type = Type::ENGINE;
	};

	inline void Modify() { ENGINE_ASSERT_WITH_MESSAGE(!m_modified, "Component last change wasn't processed");  m_modified = true; }
	inline void Reset() { m_modified = false; }
	inline bool IsModified() const { return m_modified; }

private:
	bool m_modified = false;
};

} // engine::ecs