#pragma once

#include <Engine/Config.hpp>
#include <Engine/Service/IService.hpp>
#include <Engine/Service/Project/Project.hpp>

namespace engine
{

class ENGINE_API ProjectService : public Service<ProjectService>
{
public:
    ProjectService();
    ~ProjectService();

    virtual void                        Update(float dt) override;
    virtual    void                        PostUpdate(float dt) override;

    void                                Load(const io::fs::path& path);

    const std::unique_ptr<Project>&        CurrentProject() const { ENGINE_ASSERT(m_project);  return m_project; }

private:
    std::unique_ptr<Project> m_project;
};

} // engine