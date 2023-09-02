#include "MaterialLoader.hpp"
#include "Material.hpp"
#include "AssetManager.hpp"

using namespace RightEngine;

AssetHandle MaterialLoader::Load()
{
    const auto guid = xg::newGuid();
    return _Load(guid.str(), guid);
}

AssetHandle MaterialLoader::LoadWithGUID(const xg::Guid& guid)
{
    return _Load(guid.str(), guid);
}

AssetHandle MaterialLoader::_Load(std::string_view path, const xg::Guid& guid)
{
    R_CORE_ASSERT(manager, "");
	auto asset = manager->GetAsset<Material>(path);
    if (!asset)
    {
        asset = manager->GetAsset<Material>({ guid });
    }

    if (asset)
    {
        return { asset->guid };
    }

    const auto material = std::make_shared<Material>();
    return manager->CacheAsset(material, path, AssetType::MATERIAL, guid);
}
