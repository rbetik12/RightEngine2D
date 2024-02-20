#include <Engine/Service/Project/ProjectService.hpp>
#include <Engine/Registration.hpp>

RTTR_REGISTRATION
{
	engine::registration::Service<engine::ProjectService>("engine::ProjectService");
}

namespace engine
{

ProjectService::ProjectService()
{
}

ProjectService::~ProjectService()
{
}

void ProjectService::Update(float dt)
{
}

void ProjectService::PostUpdate(float dt)
{
}

void ProjectService::Load(const io::fs::path& path)
{
	ENGINE_ASSERT(!m_project);
	m_project = std::make_unique<Project>(path);
	core::log::info("Successfully loaded project '{}'", path.generic_u8string());
}

} // engine