#pragma once

#include <Engine/Config.hpp>
#include <Engine/Domain.hpp>
#include <Core/Type.hpp>
#include <Core/RTTRIntegration.hpp>
#include <EASTL/vector.h>

namespace engine
{

class ENGINE_API IService : public core::NonCopyable
{
    RTTR_DECLARE_ROOT()
    RTTR_ENABLE_OBJECT_INFO()

public:
    struct MetaInfo
    {
        Domain m_domain = Domain::ALL;
        eastl::vector<rttr::type> m_updateAfter;
        eastl::vector<rttr::type> m_updateBefore;
        eastl::vector<rttr::type> m_postUpdateAfter;
        eastl::vector<rttr::type> m_postUpdateBefore;
    };

    virtual ~IService() = default;

    virtual void Update(float dt) {}
    virtual void PostUpdate(float dt) {}
};

template<typename T>
class Service : public IService
{
    RTTR_DECLARE_ANCESTORS(IService)
    RTTR_ENABLE_OBJECT_INFO_AS(T)
};

} // namespace engine