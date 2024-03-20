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

    if (path.is_relative())
    {
        const io::fs::path absPath = io::fs::absolute(path);
        m_project = std::make_unique<Project>(absPath);
        core::log::info("Successfully loaded project '{}'", absPath.generic_u8string());
    }
    else
    {
        m_project = std::make_unique<Project>(path);
        core::log::info("Successfully loaded project '{}'", path.generic_u8string());
    }
}

} // engine