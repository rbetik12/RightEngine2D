#include <Engine/Service/Filesystem/NativeFilesystem.hpp>

namespace engine::io
{

NativeFilesystem::NativeFilesystem(fs::path root) : IFilesystem(root)
{
}

NativeFilesystem::~NativeFilesystem()
{
}

fs::path NativeFilesystem::Absolute(const fs::path& path) const
{
	return m_fullPath.generic_u8string() + path.generic_u8string();
}

} // engine::io