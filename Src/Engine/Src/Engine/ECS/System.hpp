#pragma once

#include <Engine/Config.hpp>
#include <Engine/Domain.hpp>
#include <Core/Type.hpp>
#include <Core/RTTRIntegration.hpp>

namespace engine::ecs
{

class World;

class ENGINE_API ISystem : public core::NonCopyable
{
    RTTR_DECLARE_ROOT()
    RTTR_ENABLE_OBJECT_INFO()

public:
    struct MetaInfo
    {
        Domain m_domain = Domain::ALL;
        eastl::vector<rttr::type> m_updateAfter;
        eastl::vector<rttr::type> m_updateBefore;
    };

    ISystem(World* world);
    virtual ~ISystem() = default;

    virtual void Update(float dt) {}

    World* W() { return m_world; }

private:
    World* m_world;
};

template<typename T>
class System : public ISystem
{
    RTTR_DECLARE_ANCESTORS(ISystem)
    RTTR_ENABLE_OBJECT_INFO_AS(T)

public:
    System(World* world) : ISystem(world) {}
};

} // engine::ecs