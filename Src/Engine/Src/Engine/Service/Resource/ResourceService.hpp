#pragma once

#include <Engine/Service/Resource/Loader.hpp>
#include <Engine/Service/Resource/MaterialResource.hpp>
#include <Engine/Service/Resource/MeshResource.hpp>
#include <Engine/Service/Resource/TextureResource.hpp>
#include <Engine/Service/IService.hpp>
#include <Core/RTTRIntegration.hpp>

namespace engine
{

class MaterialResource;
class TextureResource;
class MeshResource;

class ENGINE_API ResourceService : public Service<ResourceService>
{
public:
	ResourceService();
	virtual ~ResourceService() override;

	virtual void Update(float dt) override;
	virtual void PostUpdate(float dt) override;

	void InitializeLoaders();

#pragma warning(push)
#pragma warning(disable : 4702)
	template<typename T>
	std::shared_ptr<T> Load(const io::fs::path& path)
	{
		static_assert(std::is_base_of_v<Resource, T>);

		if constexpr (std::is_same_v<T, MaterialResource>)
		{
			return LoadOf<MaterialResource, MaterialLoader>(path);
		}

		if constexpr (std::is_same_v<T, MeshResource>)
		{
			return LoadOf<MeshResource, MeshLoader>(path);
		}

		if constexpr (std::is_same_v<T, TextureResource>)
		{
			return LoadOf<TextureResource, TextureLoader>(path);
		}

		ENGINE_ASSERT_WITH_MESSAGE(false, fmt::format("Unknown resource type: '{}'", rttr::type::get<T>().get_name()));
		return {};
	}
#pragma warning(pop) 

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
	template<typename TResource, typename TLoader>
	std::shared_ptr<TResource> LoadOf(const io::fs::path& path)
	{
		static_assert(std::is_base_of_v<Loader, TLoader>);

		auto loaderIt = m_loadersMap.find(rttr::type::get<TLoader>());
		ENGINE_ASSERT(loaderIt != m_loadersMap.end());

		auto& loader = loaderIt->second;
		ENGINE_ASSERT(loader);

		return std::static_pointer_cast<TResource>(loader->Load(path));
	}

	eastl::unordered_map<rttr::type, std::unique_ptr<Loader>> m_loadersMap;
};

}