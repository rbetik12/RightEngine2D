#include <Engine/Service/Filesystem/File.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Engine.hpp>
#include <fstream>

namespace engine::io
{

File::File(const fs::path& path) : m_path(path)
{
}

bool File::Read()
{
    auto& vfs = Instance().Service<VirtualFilesystemService>();

    std::ifstream file(vfs.Absolute(m_path).c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    eastl::vector<char> fileBuffer(size);
    if (!file.read(fileBuffer.data(), size))
    {
        return false;
    }

    m_data = core::Blob(fileBuffer.data(), fileBuffer.size());
    return true;
}

} // engine::io