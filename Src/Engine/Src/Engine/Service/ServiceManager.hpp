#pragma once

#include <Engine/Config.hpp>
#include <Engine/Assert.hpp>
#include <Engine/Service/IService.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Timer.hpp>
#include <Core/Type.hpp>
#include <Core/EASTLIntergration.hpp>
#include <Core/RTTRIntegration.hpp>
#include <Core/Log.hpp>
#include <EASTL/unordered_map.h>
#include <EASTL/vector_map.h>
#include <EASTL/vector_set.h>
#include <fmt/format.h>
#include <rttr/type>

namespace engine
{

class ENGINE_API ServiceManager : public core::NonCopyable
{
public:
    ServiceManager(Domain engineDomain) : m_domain(engineDomain)
    {}

    template<typename T>
    bool RegisterService()
    {
        static_assert(std::is_base_of_v<IService, T>, "T must be derived of engine::IService");

        const auto type = rttr::type::get<T>();
        ENGINE_ASSERT_WITH_MESSAGE(type.get_constructor().is_valid(), fmt::format("Type '{}' must be registered in rttr", type.get_name()));

        if (m_servicesMap.find(type) == m_servicesMap.end())
        {
            const auto metadata = type.get_metadata(registration::C_METADATA_KEY).get_value<IService::MetaInfo>();

            if (metadata.m_domain != Domain::ALL && (m_domain & metadata.m_domain) != metadata.m_domain)
            {
                core::log::info("[ServiceManager] Skipping '{}' registration due to incompatible domain '{}' engine domain: '{}'", 
                    type.get_name(), 
                    DomainToString(metadata.m_domain),
                    DomainToString(m_domain));
                return false;
            }

            core::log::info("[ServiceManager] Started registration of service '{}'", type.get_name());
            Timer timer;

            auto service = std::make_shared<T>();

            const auto creationTimeMs = timer.TimeInMilliseconds();

            m_services.push_back(service);
            m_servicesMap[type] = m_services.size() - 1;

            core::log::info("[ServiceManager] Registered service '{}' for {}ms successfully", type.get_name(), creationTimeMs);
            return true;
        }
        ENGINE_ASSERT(false);
        return false;
    }

    template <class T>
    T& Service()
    {
        static_assert(std::is_base_of_v<IService, T>, "T must be derived of engine::IService");

        const auto type = rttr::type::get<T>();
        const auto serviceIt = m_servicesMap.find(type);
        if (serviceIt == m_servicesMap.end())
        {
            static std::shared_ptr<T> empty;
            ENGINE_ASSERT_WITH_MESSAGE(false, fmt::format("[ServiceManager] Service {} doesn't exist!", type.get_name()));
            return *empty;
        }

        const auto serviceIdx = serviceIt->second;
        auto* s = static_cast<T*>(m_services.at(serviceIdx).get());
        return *s;
    }

    void Update(float dt);
    void PostUpdate(float dt);

    void UpdateDependencyOrder();

    void Destroy();

private:
    eastl::unordered_map<rttr::type, size_t>    m_servicesMap;
    eastl::vector<std::shared_ptr<IService>>    m_services;
    eastl::vector<std::shared_ptr<IService>>    m_updateOrder;
    eastl::vector<std::shared_ptr<IService>>    m_postUpdateOrder;
    Domain                                      m_domain;
};

} // namespace engine