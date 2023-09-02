#pragma once

#include "AssetBase.hpp"
#include "AssetLoader.hpp"

namespace RightEngine
{
    class MaterialLoader : public AssetLoader
    {
    public:
        AssetHandle Load();
        AssetHandle LoadWithGUID(const xg::Guid& guid);

    private:
        AssetHandle _Load(std::string_view path, const xg::Guid& guid);
    };
}