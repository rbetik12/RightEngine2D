#include <Engine/Service/Filesystem/IFilesystem.hpp>

namespace engine::io
{

IFilesystem::IFilesystem(fs::path root) : m_fullPath(root)
{
}

} // engine::io