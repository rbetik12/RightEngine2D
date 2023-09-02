#pragma once

#include "Texture.hpp"
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <memory>

namespace RightEngine
{
    struct MaterialData
    {
        glm::vec4 albedo{ 1.0f, 1.0f, 1.0f, 1.0f };
        float metallic{ 0.0f };
        float roughness{ 1.0f };
    };

    struct TextureData
    {
        TextureData();

        AssetHandle albedo;
        AssetHandle normal;
        AssetHandle metallic;
        AssetHandle roughness;
        AssetHandle ao;
    };

    struct Material : public AssetBase
    {
        ASSET_BASE()

        TextureData textureData;
        MaterialData materialData;
    };
}
