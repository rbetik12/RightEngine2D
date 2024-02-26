#pragma once

#include <Engine/Service/Filesystem/IFilesystem.hpp>

namespace engine::io
{

class ENGINE_API NativeFilesystem : public IFilesystem
{
public:
    NativeFilesystem(const fs::path& alias, const fs::path& root);

    virtual ~NativeFilesystem() override;

    virtual fs::path Absolute(const fs::path& path) const override;

private:
};

} // engine::io