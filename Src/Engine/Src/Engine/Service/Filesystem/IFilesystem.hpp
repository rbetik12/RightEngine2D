#pragma once

#include <Engine/Config.hpp>
#include <filesystem>

namespace engine::io
{

namespace fs = std::filesystem;

class ENGINE_API IFilesystem
{
public:
	IFilesystem(fs::path root);

	virtual ~IFilesystem() {}

	virtual fs::path Absolute(const fs::path& path) const = 0;

protected:
	fs::path m_fullPath;
};

} // engine::io