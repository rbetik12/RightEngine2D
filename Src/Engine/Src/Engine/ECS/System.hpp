#pragma once

#include <Engine/Config.hpp>
#include <Engine/Domain.hpp>
#include <Core/Type.hpp>
#include <Core/RTTRIntegration.hpp>

namespace engine::ecs
{

class World;

class ENGINE_API System : public core::NonCopyable
{
	RTTR_ENABLE()
public:
	struct MetaInfo
	{
		Domain m_domain = Domain::ALL;
		eastl::vector<rttr::type> m_updateAfter;
		eastl::vector<rttr::type> m_updateBefore;
	};

	System(World* world);
	virtual ~System() = default;

	virtual void Update(float dt) = 0;

	const World* W() const { return m_world; }

private:
	World* m_world;
};

} // engine::ecs