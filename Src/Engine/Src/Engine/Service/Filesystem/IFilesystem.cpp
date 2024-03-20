#include <Engine/Service/Filesystem/IFilesystem.hpp>
#include <Engine/Engine.hpp>

namespace engine::io
{

IFilesystem::IFilesystem(const fs::path& alias, const fs::path& root) : m_alias(alias)
{
	m_fullPath = Instance().Cfg().m_projectPath.parent_path().parent_path().parent_path().generic_u8string() + root.generic_u8string();
}

} // engine::io