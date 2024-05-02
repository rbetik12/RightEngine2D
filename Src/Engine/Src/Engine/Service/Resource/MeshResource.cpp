#include <Engine/Service/Resource/MeshResource.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

RTTR_REGISTRATION
{
	engine::registration::Class<engine::MeshLoader>("engine::MeshLoader");
}

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

std::shared_ptr<engine::render::SubMesh> BuildSubMesh(const eastl::vector<Vertex>& vertices,
                                                      const eastl::vector<uint32_t>& indexes,
                                                      const engine::io::fs::path& path, 
	                                                  size_t index)
{
	if (vertices.empty())
	{
		ENGINE_ASSERT(false);
		return {};
	}

	auto& rs = engine::Instance().Service<engine::RenderService>();
	engine::RPtr<rhi::Buffer> vb;
	engine::RPtr<rhi::Buffer> ib;

	rhi::BufferDescriptor vertexBufferDescriptor{};
	vertexBufferDescriptor.m_type = rhi::BufferType::VERTEX;
	vertexBufferDescriptor.m_size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
	vertexBufferDescriptor.m_memoryType = rhi::MemoryType::CPU_GPU;
	vertexBufferDescriptor.m_name = fmt::format("SubMesh VB #{} | '{}'", index, path.generic_u8string());
	vb = rs.CreateBuffer(vertexBufferDescriptor, vertices.data());

	if (!indexes.empty())
	{
		rhi::BufferDescriptor indexBufferDescriptor{};
		indexBufferDescriptor.m_type = rhi::BufferType::INDEX;
		indexBufferDescriptor.m_size = static_cast<uint32_t>(indexes.size() * sizeof(uint32_t));
		indexBufferDescriptor.m_memoryType = rhi::MemoryType::CPU_GPU;
		indexBufferDescriptor.m_name = fmt::format("SubMesh IB #{} | '{}'", index, path.generic_u8string());
		ib = rs.CreateBuffer(indexBufferDescriptor, indexes.data());
	}

	return std::make_shared<engine::render::SubMesh>(vb, ib);
}

} // unnamed

namespace engine
{

MeshLoader::MeshLoader()
{
}

void MeshLoader::Update()
{
	PROFILER_CPU_ZONE;
}

ResPtr<Resource> MeshLoader::Load(const fs::path& path)
{
	std::lock_guard l(m_mutex);

	if (auto res = Get(path))
	{
		return res;
	}

	auto resource = MakeResPtr<MeshResource>(path);
	resource->m_status = Resource::Status::LOADING;
	m_cache[path] = resource;

	auto& ts = Instance().Service<ThreadService>();

	ts.AddBackgroundTask([this, resource]()
		{
			PROFILER_CPU_ZONE_NAME("Load mesh");
	        const auto result = Load(resource);
	        resource->m_status = result ? Resource::Status::READY : Resource::Status::FAILED;
		});

	return resource;
}

ResPtr<Resource> MeshLoader::Get(const fs::path& path) const
{
	if (const auto it = m_cache.find(path); it != m_cache.end())
	{
		return it->second;
	}
	return {};
}

bool MeshLoader::Load(const ResPtr<MeshResource>& resource)
{
	const aiScene* scene = nullptr;

	{
		std::lock_guard l(m_mutex);
		PROFILER_CPU_ZONE_NAME("Import mesh");

		auto& vfs = Instance().Service<io::VirtualFilesystemService>();

		scene = m_importer.ReadFile(vfs.Absolute(resource->m_srcPath).generic_u8string(),
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
			core::log::error("[MeshLoader] Error in loading mesh: '{}'. Message: '{}'", resource->m_srcPath.generic_u8string(), m_importer.GetErrorString());
			return false;
		}

		scene = m_importer.GetOrphanedScene();
	}

	resource->m_mesh = std::make_shared<render::Mesh>();
	ProcessAiNode(scene->mRootNode, scene, resource);
	delete scene;
	return true;
}

void MeshLoader::ProcessAiNode(const aiNode* node, const aiScene* scene, const ResPtr<MeshResource>& resource)
{
	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessAiMesh(mesh, resource);
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessAiNode(node->mChildren[i], scene, resource);
	}
}

void MeshLoader::ProcessAiMesh(const aiMesh* mesh, const ResPtr<MeshResource>& resource)
{
	eastl::vector<Vertex> vertices;
	eastl::vector<uint32_t> indexes;

	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex{};

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
			ENGINE_ASSERT(false);
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

	auto builtMesh = BuildSubMesh(vertices, indexes, resource->SourcePath(), resource->m_mesh->GetSubMeshList().size());
	resource->m_mesh->AddSubMesh(builtMesh);
}

MeshResource::MeshResource(const io::fs::path& path) : Resource(path)
{
}

} // engine