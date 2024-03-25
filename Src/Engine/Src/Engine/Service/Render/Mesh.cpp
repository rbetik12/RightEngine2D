#include <Engine/Service/Render/Mesh.hpp>

namespace engine
{

void render::Mesh::AddSubMesh(const std::shared_ptr<SubMesh>& submesh)
{
	ENGINE_ASSERT(eastl::find(m_submeshes.begin(), m_submeshes.end(), submesh) == m_submeshes.end());

	m_submeshes.emplace_back(submesh);
}

} // engine