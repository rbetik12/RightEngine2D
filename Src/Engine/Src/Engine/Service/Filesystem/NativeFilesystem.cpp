#include <Engine/Service/Filesystem/NativeFilesystem.hpp>

namespace engine::io
{

NativeFilesystem::NativeFilesystem(const fs::path& alias, const fs::path& root) : IFilesystem(alias, root)
{
}

NativeFilesystem::~NativeFilesystem()
{
}

fs::path NativeFilesystem::Absolute(const fs::path& path) const
{
    auto p = path.generic_u8string();

    p.erase(0, m_alias.generic_u8string().size());

    io::fs::path pth = m_fullPath.generic_u8string() + "/" + p;
    pth = pth.lexically_normal();

    return pth;
}

} // engine::io