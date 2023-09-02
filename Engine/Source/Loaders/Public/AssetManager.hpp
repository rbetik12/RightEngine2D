#pragma once

#include "Texture.hpp"
#include "MeshLoader.hpp"
#include "Shader.hpp"
#include "AssetLoader.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <shared_mutex>

namespace RightEngine
{
    class AssetManager
    {
    public:
        static AssetManager& Get();

        template<class T>
        void RegisterLoader(const std::shared_ptr<AssetLoader>& loader)
        {
            R_CORE_ASSERT(static_cast<bool>(std::is_base_of_v<AssetLoader, T>), "");
            const auto typeIndex = std::type_index(typeid(T));
            const auto loaderIt = loaders.find(typeIndex);
            if (loaderIt == loaders.end())
            {
                loaders[typeIndex] = loader;
                loader->OnRegister(this);
                return;
            }
            R_CORE_ASSERT(false, "")
        }

        template<class T>
        std::shared_ptr<T> GetLoader() const
        {
            R_CORE_ASSERT(static_cast<bool>(std::is_base_of_v<AssetLoader, T>), "");
            const auto typeIndex = std::type_index(typeid(T));
            const auto loaderIt = loaders.find(typeIndex);
            if (loaderIt == loaders.end())
            {
                R_CORE_WARN("No loaders are registered for asset type {0}", typeid(T).name());
                return nullptr;
            }
            return std::static_pointer_cast<T>(loaderIt->second);
        }

        template<class T>
        std::shared_ptr<T> GetAsset(std::string_view path)
        {
            R_CORE_ASSERT(!path.empty(), "");
            R_CORE_ASSERT(static_cast<bool>(std::is_base_of_v<AssetBase, T>), "");
            auto guidIt = guidCache.find(path.data());
            if (guidIt == guidCache.end())
            {
                return nullptr;
            }
            return GetAsset<T>({ guidIt->second });
        }

        template<class T>
        std::shared_ptr<T> GetAsset(const AssetHandle& assetHandle)
        {
            std::shared_lock lock(m_assetCacheMutex);
            R_CORE_ASSERT(assetHandle.guid.isValid(), "");
            R_CORE_ASSERT(static_cast<bool>(std::is_base_of_v<AssetBase, T>), "");
            auto assetIt = assetCache.find(assetHandle.guid);
            if (assetIt == assetCache.end())
            {
                return nullptr;
            }
            auto ptr = std::dynamic_pointer_cast<T>(assetIt->second);
            R_CORE_ASSERT(ptr != nullptr, "");
            return ptr;
        }

        template<class T>
        AssetHandle CacheAsset(const std::shared_ptr<T>& ptr, std::string_view path, AssetType type, const xg::Guid& guid = {})
        {
            std::lock_guard lock(m_assetCacheMutex);
            R_CORE_ASSERT(static_cast<bool>(std::is_base_of_v<AssetBase, T>), "");
            auto basePtr = std::dynamic_pointer_cast<AssetBase>(ptr);
            R_CORE_ASSERT(basePtr != nullptr, "");
            if (guid.isValid())
            {
                basePtr->guid = guid;
            }
            else
            {
                basePtr->guid = xg::newGuid();
            }
            basePtr->type = type;
            basePtr->path = path;
            assetCache[basePtr->guid] = basePtr;
            guidCache[path.data()] = basePtr->guid;
            return { basePtr->guid };
        }

        const AssetHandle& GetDefaultTexture() const;
        const AssetHandle& GetDefaultMaterial() const;
        const AssetHandle& GetDefaultSkybox() const;

        AssetManager(const AssetManager& other) = delete;
        AssetManager& operator=(const AssetManager& other) = delete;
        AssetManager(AssetManager&& other) = delete;
        AssetManager& operator=(AssetManager&& other) = delete;

    private:
        std::unordered_map<xg::Guid, std::shared_ptr<AssetBase>> assetCache;
        std::unordered_map<std::string, xg::Guid> guidCache;
        std::unordered_map<std::type_index, std::shared_ptr<AssetLoader>> loaders;
        std::shared_mutex m_assetCacheMutex;

        mutable AssetHandle defaultTexture;
        mutable AssetHandle defaultMaterial;
        mutable AssetHandle defaultSkybox;

        AssetManager() = default;
        ~AssetManager() = default;

        void RemoveAsset(const AssetHandle& handle)
        {
            assetCache.erase(handle.guid);
        }

        friend class AssetLoader;
    };
}
