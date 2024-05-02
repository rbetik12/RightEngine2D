#pragma once

#include <Engine/Service/Resource/Resource.hpp>
#include <Engine/Service/Resource/Loader.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <RHI/Texture.hpp>
#include <taskflow/taskflow.hpp>

namespace engine
{

class MaterialResource;

class ENGINE_API MaterialLoader final : public Loader
{
	RTTR_ENABLE(Loader)
public:
	MaterialLoader();

	virtual void					Update() override;

	virtual ResPtr<Resource>		Load(const fs::path& path) override;

	virtual ResPtr<Resource>		Get(const fs::path& path) const override;

	virtual void					LoadSystemResources() override;

	const ResPtr<rhi::Pipeline>&	Pipeline(const ResPtr<MaterialResource>& res) const;

	// Called automatically, don't call it unless you know what are you doing!!!
	void							ResizePipelines(glm::ivec2 extent, bool onScreen = false);

private:
	struct ParsedPipelineInfo
	{
		eastl::vector<rhi::AttachmentDescriptor>	m_attachments;
		rhi::AttachmentDescriptor					m_depthAttachment;
		std::shared_ptr<rhi::Shader>				m_shader;
		glm::ivec2									m_viewportSize = { 0, 0 };
		bool										m_compute = false;
		bool										m_offscreen = true; // ignored in compute
		rhi::CompareOp								m_depthCompareOp = rhi::CompareOp::LESS; // ignored in compute
		rhi::CullMode								m_cullMode = rhi::CullMode::BACK; // ignored in compute
	};

	struct ParsedMaterial
	{
		io::fs::path		m_shaderPath;
		std::string			m_name;
		uint8_t				m_version = std::numeric_limits<uint8_t>::max();
		ParsedPipelineInfo	m_parsedPipeline;
	};

	bool							Load(const ResPtr<MaterialResource>& resource);
	ParsedMaterial					ParseJson(std::ifstream& stream);
	std::shared_ptr<rhi::Pipeline>	AllocatePipeline(ParsedPipelineInfo& info);

	std::mutex																			m_mutex;
	eastl::vector<tf::Future<void>>														m_loadingTasks;
	std::shared_ptr<rhi::ShaderCompiler>												m_shaderCompiler;
	eastl::unordered_map<io::fs::path, std::shared_ptr<rhi::Shader>>					m_shaderCache;
	eastl::unordered_map<std::shared_ptr<rhi::Shader>, std::shared_ptr<rhi::Pipeline>>	m_shaderToPipeline;
	eastl::unordered_map<fs::path, ResPtr<MaterialResource>>							m_cache;
};

class ENGINE_API MaterialResource final : public Resource
{
public:
	MaterialResource(const io::fs::path& path);
	virtual ~MaterialResource() {}

	const ResPtr<render::Material>& Material() const { ENGINE_ASSERT(Ready()); return m_material; }

	friend class MaterialLoader;

private:
	ResPtr<render::Material> m_material;
};

} // engine