#include "AssetLoader.hpp"
#include "AssetManager.hpp"

using namespace RightEngine;

void AssetLoader::OnRegister(AssetManager* aManager)
{
    manager = aManager;
}

void AssetLoader::RemoveAsset(const AssetHandle& handle)
{
    R_CORE_ASSERT(manager, "")
    manager->RemoveAsset(handle);
}

void AssetLoader::WaitAllLoaders()
{
    //taskGroup.wait();
}
