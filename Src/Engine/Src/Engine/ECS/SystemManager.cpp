#include <Engine/ECS/SystemManager.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Registration.hpp>
#include <Core/RTTRIntegration.hpp>
#include <EASTL/algorithm.h>

namespace engine::ecs
{

SystemManager::SystemManager(World* world) : m_world(world)
{
}

SystemManager::~SystemManager()
{
}

void SystemManager::Update(float dt)
{
    PROFILER_CPU_ZONE;
    m_execParams.dt = dt;
    auto& ts = Instance().Service<ThreadService>();
    ts.AddForegroundTaskflow(m_taskflow).wait();
}

void SystemManager::UpdateDependenciesOrder()
{
    eastl::vector_map<rttr::type, eastl::vector<rttr::type>> dependencies;

    // Gather type deps map
    for (const auto& systemPtr : m_systems)
    {
        System& sys = *systemPtr.get();
        const rttr::instance instance = sys;
        const auto sysType = instance.get_derived_type();

        auto meta = sysType.get_metadata(engine::registration::C_METADATA_KEY).get_value<System::MetaInfo>();

        dependencies[sysType] = meta.m_updateBefore;

        for (const auto depType : meta.m_updateAfter)
        {
            dependencies[depType].emplace_back(sysType);
        }
    }

    // Build new taskflow
    tf::Taskflow tf;
    eastl::vector_map<rttr::type, tf::Task> m_tasksMap;

    // Create a task for each system update
    for (const auto& [type, _] : dependencies)
    {
        const auto system = m_typeToSystem[type];
        auto task = tf.emplace([this, system]
            {
                system->Update(m_execParams.dt);
            });

        task.name(rttr::instance(system).get_derived_type().get_name().data());
        m_tasksMap[type] = task;
    }

    // Setup tasks dependencies
    for (const auto& [type, deps] : dependencies)
    {
        auto task = m_tasksMap[type];
        for (const auto dep : deps)
        {
            task.precede(m_tasksMap[dep]);
        }
    }

    m_taskflow = std::move(tf);
}

} // engine::ecs