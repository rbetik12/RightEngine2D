#include "MeshBuilder.hpp"
#include "AssetManager.hpp"

using namespace RightEngine;

AssetHandle MeshBuilder::Cube()
{
    if (!cube.wasLoaded)
    {
        cube.handle = AssetManager::Get().GetLoader<MeshLoader>()->Load("/Models/default_cube.fbx");
    }

    return cube.handle;
}

DefaultMesh MeshBuilder::cube = {};