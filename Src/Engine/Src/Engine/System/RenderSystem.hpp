#pragma once

#include <Engine/Config.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/World.hpp>
#include <Engine/ECS/Component.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <Engine/Service/Render/Mesh.hpp>

namespace engine
{

struct ENGINE_API MeshComponent : public ecs::Component
{
	std::shared_ptr<render::Material>	m_material;
	std::shared_ptr<render::Mesh>		m_mesh;
};

class ENGINE_API RenderSystem : public ecs::System
{
	RTTR_ENABLE(System)
public:
	RenderSystem(ecs::World* world);
	virtual ~RenderSystem() = default;

	virtual void Update(float dt) override;

private:
};

} // engine