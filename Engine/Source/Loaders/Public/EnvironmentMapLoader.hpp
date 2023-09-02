#pragma once

#include "Texture.hpp"
#include "AssetLoader.hpp"
#include <string>

namespace RightEngine
{
    struct EnvironmentContext : public AssetBase
    {
        ASSET_BASE()

        std::shared_ptr<Texture> envMap;
        std::shared_ptr<Texture> irradianceMap;
        std::shared_ptr<Texture> prefilterMap;
        std::shared_ptr<Texture> brdfLut;

        //TODO: Add EDITOR compile flag
        std::shared_ptr<Texture> equirectangularTexture;
        std::string name;
    };

    struct EnvironmentMapLoaderContext
    {
        std::string path;
        TextureDescriptor specification;
    };

    class EnvironmentMapLoader : public AssetLoader
    {
    public:
        EnvironmentMapLoader();
        ~EnvironmentMapLoader() = default;

        AssetHandle Load(const std::string& path, bool flipVertically = false);
        AssetHandle LoadWithGUID(const std::string& path, const xg::Guid& guid, bool flipVertically = false);

    private:
        void ComputeEnvironmentMap();
        void ComputeIrradianceMap();
        void ComputeRadianceMap();
        void ComputeLUT();
        AssetHandle FinishLoading(const xg::Guid& guid);
        AssetHandle _Load(const std::string& path, const xg::Guid& guid, bool flipVertically);

        std::shared_ptr<EnvironmentContext> m_environmentContext;
        EnvironmentMapLoaderContext m_loaderContext;
        std::shared_ptr<Texture> m_lut;
    };
}
