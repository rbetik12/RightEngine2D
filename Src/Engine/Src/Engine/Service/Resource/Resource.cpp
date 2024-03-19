#include <Engine/Service/Resource/Resource.hpp>

namespace engine
{

Resource::Resource(const io::fs::path& path) : m_srcPath(path)
{
}

} // engine