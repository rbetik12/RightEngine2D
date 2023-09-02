#include "MeshLoader.hpp"
#include "Logger.hpp"
#include "Path.hpp"
#include "String.hpp"
#include "TextureLoader.hpp"
#include "AssetManager.hpp"
#include "AssetLoader.hpp"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace RightEngine;

namespace
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 tangent;
        glm::vec3 biTangent;
    };

    std::shared_ptr<Mesh> BuildMesh(const std::vector<Vertex>& vertices,
                                    const std::vector<uint32_t>& indexes)
    {
        R_CORE_ASSERT(!vertices.empty(), "");
        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(3);
        layout.Push<float>(2);
        layout.Push<float>(3);
        layout.Push<float>(3);

        auto mesh = std::make_shared<Mesh>();
        BufferDescriptor vertexBufferDescriptor{};
        vertexBufferDescriptor.type = BufferType::VERTEX;
        vertexBufferDescriptor.size = vertices.size() * sizeof(Vertex);
        vertexBufferDescriptor.memoryType = MemoryType::CPU_GPU;
        const auto vertexBuffer = Device::Get()->CreateBuffer(vertexBufferDescriptor, vertices.data());
        mesh->SetVertexBuffer(vertexBuffer, std::make_shared<VertexBufferLayout>(layout));

        if (!indexes.empty())
        {
            BufferDescriptor indexBufferDescriptor{};
            indexBufferDescriptor.type = BufferType::INDEX;
            indexBufferDescriptor.size = indexes.size() * sizeof(uint32_t);
            indexBufferDescriptor.memoryType = MemoryType::CPU_GPU;
            const auto indexBuffer = Device::Get()->CreateBuffer(indexBufferDescriptor, indexes.data());
            mesh->SetIndexBuffer(indexBuffer);
        }

        return mesh;
    }
}

AssetHandle MeshLoader::Load(const std::string& aPath)
{
    return _Load(aPath, xg::Guid());
}

void MeshLoader::ProcessNode(const aiNode* node, const aiScene* scene, std::shared_ptr<MeshNode>& meshNode)
{
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshNode->meshes.push_back(ProcessMesh(mesh, scene));
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        auto newNode = std::make_shared<MeshNode>();
        meshNode->children.push_back(newNode);
        ProcessNode(node->mChildren[i], scene, meshNode);
    }
}

std::shared_ptr<Mesh> MeshLoader::ProcessMesh(const aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indexes;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = vec;
        }
        else
        {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            glm::vec3 tangent;
            tangent.x = mesh->mTangents[i].x;
            tangent.y = mesh->mTangents[i].y;
            tangent.z = mesh->mTangents[i].z;

            glm::vec3 biTangent;
            biTangent.x = mesh->mBitangents[i].x;
            biTangent.y = mesh->mBitangents[i].y;
            biTangent.z = mesh->mBitangents[i].z;

            vertex.tangent = tangent;
            vertex.biTangent = biTangent;
        }
        else
        {
            // TODO: Implement manual tangent and biTangent calculation
            R_CORE_ASSERT(false, "");
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++)
        {
            indexes.push_back(face.mIndices[j]);
        }
    }

    auto builtMesh = BuildMesh(vertices, indexes);
    return builtMesh;
}

// Current supported path convention for model's textures is this:
//          model.file
//          textures_dir/<all_textures>

std::vector<std::shared_ptr<Texture>> MeshLoader::LoadTextures(const aiMaterial* mat, aiTextureType type)
{
    std::vector<std::shared_ptr<Texture>> textures;
    TextureLoader textureLoader;
    for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // TODO: Add / delimiter support
        auto splittedPath = String::Split(std::string(str.C_Str()), "\\");

        std::string texName = meshDir + '/' + *(splittedPath.end() - 2) + '/' + *(splittedPath.end() - 1);
        if (loadedTextures.find(texName) == loadedTextures.end())
        {
            auto& assetManager = AssetManager::Get();
            auto loader = assetManager.GetLoader<TextureLoader>();
            auto textureHandle = loader->Load(texName, {});
            const auto texture = assetManager.GetAsset<Texture>(textureHandle);
            textures.push_back(texture);
            loadedTextures[texName] = texture;
        }
        else
        {
            textures.push_back(loadedTextures[texName]);
        }
    }
    return textures;
}

AssetHandle MeshLoader::Load(const std::shared_ptr<Buffer>& vertexBuffer,
                             const std::shared_ptr<VertexBufferLayout>& layout,
                             const std::shared_ptr<Buffer>& indexBuffer)
{
    auto mesh = std::make_shared<Mesh>();
    mesh->SetVertexBuffer(vertexBuffer, layout);
    mesh->SetIndexBuffer(indexBuffer);

    auto meshNode = std::make_shared<MeshNode>();
    meshNode->meshes.push_back(mesh);
    R_CORE_ASSERT(manager, "")
    return manager->CacheAsset(meshNode, "", AssetType::MESH);
}

AssetHandle MeshLoader::LoadWithGUID(const std::string& path, const xg::Guid& guid)
{
    return _Load(path, guid);
}

AssetHandle MeshLoader::_Load(const std::string& path, const xg::Guid& guid)
{
    R_CORE_ASSERT(manager, "")

	const auto asset = manager->GetAsset<MeshNode>(path);
    if (asset)
    {
        return { asset->guid };
    }

    const size_t lastDelimIndex = path.find_last_of('/');
    std::string meshName = path.substr(lastDelimIndex, path.size() - 1 - lastDelimIndex);
    meshDir = path.substr(0, lastDelimIndex);
    Assimp::Importer importer;
    auto scene = importer.ReadFile(Path::Absolute(path),
                                   aiProcess_Triangulate
                                   | aiProcess_GenSmoothNormals
                                   | aiProcess_FlipUVs
                                   | aiProcess_CalcTangentSpace
                                   | aiProcess_GenUVCoords
                                   | aiProcess_OptimizeGraph
                                   | aiProcess_OptimizeMeshes
                                   | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        R_CORE_ERROR("ASSIMP ERROR: {0}", importer.GetErrorString());
        return {};
    }

    auto meshTree = std::make_shared<MeshNode>();
    ProcessNode(scene->mRootNode, scene, meshTree);
    return manager->CacheAsset(meshTree, path, AssetType::MESH, guid);
}
