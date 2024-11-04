#pragma once

#include <Engine/Config.hpp>
#include <Engine/Service/Resource/Resource.hpp>
#include <Core/Type.hpp>

namespace engine
{

template<typename T>
using ResPtr = std::shared_ptr<T>;

template<typename T, typename... Args>
ResPtr<T> MakeResPtr(Args&&... args)
{
	static_assert(std::is_base_of_v<Resource, T>);
	return std::make_shared<T>(std::forward<Args>(args)...);
}

namespace fs = std::filesystem;

class ENGINE_API Loader : public core::NonCopyable
{
public:
	virtual ~Loader() {}

	virtual void Update() = 0;

	// Async load of a resource
	virtual ResPtr<Resource> Load(const fs::path& path) = 0;

	virtual void LoadSystemResources() = 0;

	// Return resource pointer if it was already loaded, if not returns nullptr
	virtual ResPtr<Resource> Get(const fs::path& path) const = 0;
};

} // engine