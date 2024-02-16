#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Service/Filesystem/NativeFilesystem.hpp>
#include <Engine/Registration.hpp>

RTTR_REGISTRATION
{
	engine::registration::Service<engine::io::VirtualFilesystemService>("engine::io::VirtualFilesystemService");
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
}

void VirtualFilesystemService::PostUpdate(float dt)
{
}

void VirtualFilesystemService::Assign(fs::path alias, fs::path rootPath, FilesystemType type)
{
	ENGINE_ASSERT(m_filesystems.find(alias) == m_filesystems.end());

	if (type == FilesystemType::NATIVE)
	{
		m_filesystems[alias] = std::make_unique<NativeFilesystem>(rootPath);
	}
	else
	{
		ENGINE_ASSERT(false);
	}
}

fs::path VirtualFilesystemService::Absolute(fs::path path) const
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
	}

	if (const auto aliasIt = m_filesystems.find(aliases[0] + "/" + aliases[1]); aliasIt != m_filesystems.end())
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