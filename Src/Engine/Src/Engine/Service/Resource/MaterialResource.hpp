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
	void							ResizePipelines(glm::ivec2 extent, bool offscreen = true);

	const ResPtr<MaterialResource>& RenderMaterial() const { return m_renderMaterial; }
	const ResPtr<MaterialResource>& PresentMaterial() const { return m_presentMaterial; }

	struct LoadEnvironmentMapData
	{
		ResPtr<MaterialResource>	m_material;
		RPtr<rhi::Texture>			m_cubemap;
		RPtr<rhi::Texture>			m_irradianceTexture;
		RPtr<rhi::Texture>			m_prefilterTexture;
		RPtr<rhi::Texture>			m_brdfTexture;
	};

	LoadEnvironmentMapData			LoadEnvironmentMap(const fs::path& path);

private:
	struct LoadAttachmentDescriptor
	{
		rhi::AttachmentDescriptor m_descriptor;
		fs::path m_dependency;
		int m_depAttachmentIndex;
	};

	struct ParsedPipelineInfo
	{
		eastl::vector<LoadAttachmentDescriptor>		m_attachments;
		std::optional<LoadAttachmentDescriptor>		m_depthAttachment;
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

	bool							Load(const ResPtr<MaterialResource>& resource, bool forcePipelineRecreation = false);
	ParsedMaterial					ParseJson(std::ifstream& stream);
	std::shared_ptr<rhi::Pipeline>	AllocatePipeline(ParsedPipelineInfo& info);

	mutable std::mutex																	m_mutex;
	eastl::vector<tf::Future<void>>														m_loadingTasks;
	std::shared_ptr<rhi::ShaderCompiler>												m_shaderCompiler;
	eastl::unordered_map<io::fs::path, std::shared_ptr<rhi::Shader>>					m_shaderCache;
	eastl::unordered_map<std::shared_ptr<rhi::Shader>, std::shared_ptr<rhi::Pipeline>>	m_shaderToPipeline;
	eastl::unordered_map<fs::path, ResPtr<MaterialResource>>							m_cache;
	ResPtr<MaterialResource>															m_renderMaterial;
	ResPtr<MaterialResource>															m_presentMaterial;
	ResPtr<MaterialResource>															m_skyboxMaterial;
	ResPtr<MaterialResource>															m_equirectToCubemapMaterial;
	ResPtr<MaterialResource>															m_envmapIrradianceMaterial;
	ResPtr<MaterialResource>															m_irradianceLoadMaterial;
	ResPtr<MaterialResource>															m_prefilterLoadMaterial;
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