#include <Engine/Service/Resource/Loader.hpp>

namespace engine
{

Loader::~Loader()
{
	m_cache.clear();
}

ResPtr<Resource> Loader::Exists(const fs::path& path)
{
	if (const auto it = m_cache.find(path); it != m_cache.end())
	{
		return it->second;
	}
	return {};
}

} // engine