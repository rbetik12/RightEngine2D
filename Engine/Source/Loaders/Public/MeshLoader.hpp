#pragma once

#include "AssetBase.hpp"
#include "Components.hpp"
#include <assimp/scene.h>
#include <vector>
#include <unordered_map>

namespace RightEngine
{
    class Mesh
    {
    public:
        const std::shared_ptr<Buffer>& GetVertexBuffer() const
        { return vertexBuffer; }
        void SetVertexBuffer(const std::shared_ptr<Buffer>& aVertexBuffer, const std::shared_ptr<VertexBufferLayout>& aLayout)
        {
            vertexBuffer = aVertexBuffer;
            vertexLayout = aLayout;
        }

        const std::shared_ptr<Buffer>& GetIndexBuffer() const
        { return indexBuffer; }
        void SetIndexBuffer(const std::shared_ptr<Buffer>& anIndexBuffer)
        { indexBuffer = anIndexBuffer; }

        const std::shared_ptr<VertexBufferLayout>& GetVertexLayout() const
        { return vertexLayout; }

    private:
        std::shared_ptr<Buffer> vertexBuffer;
        std::shared_ptr<Buffer> indexBuffer;
        std::shared_ptr<VertexBufferLayout> vertexLayout;
    };

    struct MeshNode : public AssetBase
    {
        ASSET_BASE()

        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<std::shared_ptr<MeshNode>> children;
    };

    class MeshLoader : public AssetLoader
    {
    public:
        MeshLoader() = default;
        ~MeshLoader() = default;

        AssetHandle Load(const std::string& path);
        AssetHandle Load(const std::shared_ptr<Buffer>& vertexBuffer,
                         const std::shared_ptr<VertexBufferLayout>& layout,
                         const std::shared_ptr<Buffer>& indexBuffer = nullptr);

        AssetHandle LoadWithGUID(const std::string& path, const xg::Guid& guid);

    private:
        std::string meshDir;
        std::unordered_map<std::string, std::shared_ptr<Texture>> loadedTextures;

        void ProcessNode(const aiNode* node, const aiScene* scene, std::shared_ptr<MeshNode>& meshNode);
        std::shared_ptr<Mesh> ProcessMesh(const aiMesh* mesh, const aiScene* scene);
        std::vector<std::shared_ptr<Texture>> LoadTextures(const aiMaterial* mat, aiTextureType type);
        AssetHandle _Load(const std::string& path, const xg::Guid& guid);
    };
}