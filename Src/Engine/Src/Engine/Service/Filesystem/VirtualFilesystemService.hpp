#pragma once

#include <Engine/Service/IService.hpp>
#include <Engine/Service/Filesystem/IFilesystem.hpp>
#include <EASTL/unordered_map.h>

namespace engine::io
{

enum class FilesystemType : uint8_t
{
	NATIVE = 0
};

class ENGINE_API VirtualFilesystemService : public IService
{
	RTTR_ENABLE(IService);
public:
	VirtualFilesystemService();

	virtual ~VirtualFilesystemService() override;

	virtual void	Update(float dt) override;
	virtual void	PostUpdate(float dt) override;

	void			Assign(fs::path alias, fs::path rootPath, FilesystemType type = FilesystemType::NATIVE);

	fs::path		Absolute(fs::path path) const;

private:
	eastl::unordered_map<fs::path, std::unique_ptr<IFilesystem>> m_filesystems;
};

} // engine::io