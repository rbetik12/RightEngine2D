#pragma once

#include <Engine/Config.hpp>
#include <Engine/ECS/System.hpp>
#include <Core/Type.hpp>
#include <taskflow/taskflow.hpp>

namespace engine::ecs
{

class ENGINE_API SystemManager : public core::NonCopyable
{
public:
	SystemManager(World* world);
	~SystemManager();

	void Update(float dt);

	template<typename T>
	void Add()
	{
		static_assert(std::is_base_of_v<System, T>);
		ENGINE_ASSERT(rttr::type::get<T>().get_constructor({rttr::type::get<World*>()}).is_valid());

		m_systems.emplace_back(std::make_unique<T>(m_world));
		m_typeToSystem[rttr::type::get<T>()] = m_systems.back().get();
	}

	void UpdateDependenciesOrder();

private:
	struct ExecutionParams
	{
		float dt;
	};

	eastl::vector<std::unique_ptr<System>>						m_systems;
	eastl::unordered_map<rttr::type, System*>					m_typeToSystem;
	World*														m_world;
	tf::Taskflow												m_taskflow;
	ExecutionParams												m_execParams;
};

} // engine::ecs