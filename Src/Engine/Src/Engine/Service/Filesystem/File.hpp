#pragma once

#include <Engine/Service/Filesystem/IFilesystem.hpp>
#include <Core/Blob.hpp>

namespace engine::io
{

class ENGINE_API File
{
public:
	File(const fs::path& path);

	bool				Read();

	const fs::path&		Path() const { return m_path; }
	size_t				Size() const { return m_data.size(); }
	void*				Raw() { return m_data.raw(); }

private:
	fs::path	m_path;
	core::Blob	m_data;
};

} // engine::io