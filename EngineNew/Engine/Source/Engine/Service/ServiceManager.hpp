#pragma once

#include <Engine/Config.hpp>
#include <Engine/Assert.hpp>
#include <Engine/Service/IService.hpp>
#include <Core/Type.hpp>
#include <Core/EASTLIntergration.hpp>
#include <Core/RTTRIntegration.hpp>
#include <Core/Log.hpp>
#include <EASTL/unordered_map.h>
#include <fmt/format.h>
#include <rttr/type>
#include <typeindex>

namespace engine
{

class ENGINE_API ServiceManager : public core::NonCopyable
{
public:
	template<typename T>
	bool RegisterService()
	{
        static_assert(std::is_base_of_v<IService, T>, "T must be derived of engine::IService");

        const auto typeIndex = std::type_index(typeid(T));
        ENGINE_ASSERT_WITH_MESSAGE(rttr::type::get<T>().get_constructor().is_valid(), fmt::format("Type '{}' must be registered in rttr", typeIndex.name()));

        if (m_services.find(typeIndex) == m_services.end())
        {
            auto service = std::make_shared<T>();
            m_services[typeIndex] = service;
            core::log::info("[ServiceManager] Registered service '{}' successfully", rttr::type::get<T>().get_name());
            return true;
        }
        return false;
	}

    template <class T>
    T& Service()
    {
        static_assert(std::is_base_of_v<IService, T>, "T must be derived of engine::IService");

        const auto typeIndex = std::type_index(typeid(T));
        const auto serviceIt = m_services.find(typeIndex);
        if (serviceIt == m_services.end())
        {
            static std::shared_ptr<T> empty;
            ENGINE_ASSERT_WITH_MESSAGE(false, fmt::format("[ServiceManager] Service {} doesn't exist!", typeIndex.name()));
            return *empty;
        }

        auto* s = static_cast<T*>(serviceIt->second.get());
        return *s;
    }

    void Update(float dt);
    void PostUpdate(float dt);

    void Destroy();

private:
	eastl::unordered_map<std::type_index, std::shared_ptr<IService>> m_services;
};

} // namespace engine