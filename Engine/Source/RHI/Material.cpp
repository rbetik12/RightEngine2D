#include "Material.hpp"
#include "AssetManager.hpp"

using namespace RightEngine;

RightEngine::TextureData::TextureData()
{
    auto& assetManager = AssetManager::Get();
    albedo = assetManager.GetDefaultTexture();
    normal = assetManager.GetDefaultTexture();
    metallic = assetManager.GetDefaultTexture();
    roughness = assetManager.GetDefaultTexture();
    ao = assetManager.GetDefaultTexture();
}

