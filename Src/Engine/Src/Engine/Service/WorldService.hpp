#pragma once

#include <Engine/Config.hpp>
#include <Engine/Service/IService.hpp>
#include <Engine/ECS/World.hpp>

namespace engine
{

class ENGINE_API WorldService : public IService
{
	RTTR_ENABLE(IService);
public:
	WorldService();
	virtual ~WorldService() override;

	virtual void Update(float dt) override;
	virtual void PostUpdate(float dt) override;

	std::unique_ptr<ecs::World>& CurrentWorld() { return m_world; }

private:
	std::unique_ptr<ecs::World> m_world;
};

} // engine