#pragma once

#include <Engine/Service/Resource/Loader.hpp>
#include <Engine/Service/Render/Mesh.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <taskflow/taskflow.hpp>

namespace engine
{

class MeshResource;

class ENGINE_API MeshLoader final : public Loader
{
public:
	MeshLoader();

	virtual void				Update() override;

	virtual ResPtr<Resource>	Load(const fs::path& path) override;

	virtual ResPtr<Resource>	Get(const fs::path& path) const override;

	virtual void				LoadSystemResources() override {}

private:
	bool Load(const ResPtr<MeshResource>& resource);

	void ProcessAiNode(const aiNode* node, const aiScene* scene, const ResPtr<MeshResource>& resource);
	void ProcessAiMesh(const aiMesh* mesh, const ResPtr<MeshResource>& resource);

	std::mutex												m_mutex;
	eastl::vector<tf::Future<void>>							m_loadingTasks;
	Assimp::Importer										m_importer;
	eastl::unordered_map<fs::path, ResPtr<MeshResource>>	m_cache;
};

class ENGINE_API MeshResource final : public Resource
{
public:
	MeshResource(const io::fs::path & path);
	virtual ~MeshResource() {}

	const ResPtr<render::Mesh>& Mesh() const { return m_mesh; }

	friend class MeshLoader;

private:
	ResPtr<render::Mesh> m_mesh;
};

} // engine