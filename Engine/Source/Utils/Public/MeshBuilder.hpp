#pragma once

#include "Components.hpp"
#include <memory>

namespace RightEngine
{
    struct DefaultMesh
    {
        bool wasLoaded{ false };
        AssetHandle handle;
    };

    class MeshBuilder
    {
    public:
        static AssetHandle Cube();

    private:
        static DefaultMesh cube;
    };
}
