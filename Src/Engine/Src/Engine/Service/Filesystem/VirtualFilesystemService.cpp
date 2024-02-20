#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Service/Filesystem/NativeFilesystem.hpp>
#include <Engine/Registration.hpp>

namespace rttr
{
	template<typename T>
	struct sequential_container_mapper<eastl::vector<T>> :
	detail::sequential_container_base_dynamic_direct_access<eastl::vector<T>> {};
}

RTTR_REGISTRATION
{
	engine::registration::Service<engine::io::VirtualFilesystemService>("engine::io::VirtualFilesystemService");

	engine::registration::Class<engine::io::VFSSettings::Setting>("engine::io::VFSSettings::Setting")
		.property("alias", &engine::io::VFSSettings::Setting::m_alias)
		.property("path", &engine::io::VFSSettings::Setting::m_path);

	engine::registration::ProjectSettings<engine::io::VFSSettings>("engine::io::VFSSettings")
		.property("settings", &engine::io::VFSSettings::m_settings);
}

namespace engine::io
{

VirtualFilesystemService::VirtualFilesystemService()
{
}

VirtualFilesystemService::~VirtualFilesystemService()
{
}

void VirtualFilesystemService::Update(float dt)
{
	PROFILER_CPU_ZONE;
}

void VirtualFilesystemService::PostUpdate(float dt)
{
	PROFILER_CPU_ZONE;
}

void VirtualFilesystemService::Assign(const fs::path& alias, const fs::path& rootPath, FilesystemType type)
{
	ENGINE_ASSERT(m_filesystems.find(alias) == m_filesystems.end());

	if (type == FilesystemType::NATIVE)
	{
		m_filesystems[alias] = std::make_unique<NativeFilesystem>(alias, rootPath);
	}
	else
	{
		ENGINE_ASSERT(false);
	}
}

fs::path VirtualFilesystemService::Absolute(const fs::path& path) const
{
	eastl::array<std::string, 2> aliases;

	uint8_t i = 0;
	for (auto p : path)
	{
		if (i >= 2)
		{
			break;
		}
		aliases[i] = p.generic_u8string();
		i++;
	}

	if (const auto aliasIt = m_filesystems.find(aliases[0] + aliases[1]); aliasIt != m_filesystems.end())
	{
		return aliasIt->second->Absolute(path);
	}

	if (const auto aliasIt = m_filesystems.find("/"); aliasIt != m_filesystems.end())
	{
		return aliasIt->second->Absolute(path);
	}

	ENGINE_ASSERT(false);
	return {};
}

} // engine::io