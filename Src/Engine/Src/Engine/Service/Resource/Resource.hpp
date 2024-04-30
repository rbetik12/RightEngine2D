#pragma once

#include <Engine/Config.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Core/Type.hpp>

namespace engine
{

class ENGINE_API Resource : core::NonCopyable
{
public:
	enum class Status : uint8_t
	{
		UNKNOWN = 0,
		LOADING = 1,
		FAILED = 2,
		READY = 3
	};

	Resource(const io::fs::path& path);
	virtual ~Resource() {}

	const io::fs::path&		SourcePath() const { return m_srcPath; }
	bool					Ready() const { return m_status == Status::READY; }
	void					Wait() const { while(!Ready()); }

protected:
	io::fs::path	m_srcPath;
	Status			m_status = Status::UNKNOWN;
};

} // engine