#include <Engine/Service/Filesystem/IFilesystem.hpp>

namespace engine::io
{

IFilesystem::IFilesystem(const fs::path& alias, const fs::path& root) : m_fullPath(ROOT_DIR + root.generic_u8string()), m_alias(alias)
{
}

} // engine::io