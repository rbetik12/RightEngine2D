#pragma once

#include "crossguid/guid.hpp"
#include <string>

namespace RightEngine
{
    enum class AssetType
    {
        NONE = 0,
        IMAGE,
        MESH,
        ENVIRONMENT_MAP,
        SHADER,
        MATERIAL
    };

    class AssetBase
    {
    public:
        virtual std::string GetClass() const = 0;

    public:
        xg::Guid guid;
        std::string path;
        AssetType type;
    };

    struct AssetHandle
    {
        xg::Guid guid;
    };
}

#define ASSET_BASE() virtual std::string GetClass() const override \
                              { return typeid(*this).name(); }