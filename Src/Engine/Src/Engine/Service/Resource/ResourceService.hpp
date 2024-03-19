#pragma once

#include <Engine/Service/Resource/Loader.hpp>
#include <Engine/Service/IService.hpp>
#include <Core/RTTRIntegration.hpp>

namespace engine
{

class ENGINE_API ResourceService : public IService
{
	RTTR_ENABLE(IService);
public:
	ResourceService();
	virtual ~ResourceService() override;

	virtual void Update(float dt) override;
	virtual void PostUpdate(float dt) override;

	template<typename T>
	bool RegisterLoader()
	{
		static_assert(std::is_base_of_v<Loader, T>, "T must be derived of engine::Loader");

		const auto type = rttr::type::get<T>();
		ENGINE_ASSERT_WITH_MESSAGE(type.get_constructor().is_valid(), fmt::format("Type '{}' must be registered in rttr", type.get_name()));

		if (m_loadersMap.find(type) == m_loadersMap.end())
		{
			m_loadersMap[type] = std::make_unique<T>();
			core::log::info("[ResourceService] Registered loader '{}'", type.get_name());
			return true;
		}

		ENGINE_ASSERT(false);
		return false;
	}

	template<typename T>
	T& GetLoader()
	{
		static_assert(std::is_base_of_v<Loader, T>, "T must be derived of engine::Loader");

		const auto type = rttr::type::get<T>();
		const auto loaderIt = m_loadersMap.find(type);

	    if (loaderIt == m_loadersMap.end())
	    {
			static std::unique_ptr<T> empty;
			ENGINE_ASSERT(false);
			return *empty;
	    }

		return *static_cast<T*>(loaderIt->second.get());
	}

private:
	eastl::unordered_map<rttr::type, std::unique_ptr<Loader>> m_loadersMap;
};

}