#include <Engine/Service/Resource/MaterialResource.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/Filesystem/File.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/Window/WindowService.hpp>
#include <RHI/Helpers.hpp>
#include <nlohmann/json.hpp>

#include "RHI/Pipeline.hpp"
#include "RHI/RenderPass.hpp"

RTTR_REGISTRATION
{
	using namespace engine::registration;

	ResourceLoader<engine::MaterialLoader>("engine::MaterialLoader");

    rttr::registration::enumeration<rhi::CullMode>("rhi::CullMode")
        (
	        rttr::value("NONE", rhi::CullMode::NONE),
	        rttr::value("BACK", rhi::CullMode::BACK),
	        rttr::value("FRONT", rhi::CullMode::FRONT)
	    );

	rttr::registration::enumeration<rhi::CompareOp>("rhi::CompareOp")
		(
			rttr::value("LESS", rhi::CompareOp::LESS),
			rttr::value("LESS_OR_EQUAL", rhi::CompareOp::LESS_OR_EQUAL),
			rttr::value("GREATER", rhi::CompareOp::GREATER)
		);

	rttr::registration::enumeration<rhi::AttachmentLoadOperation>("rhi::AttachmentLoadOperation")
		(
			rttr::value("UNDEFINED", rhi::AttachmentLoadOperation::UNDEFINED),
			rttr::value("LOAD", rhi::AttachmentLoadOperation::LOAD),
			rttr::value("CLEAR", rhi::AttachmentLoadOperation::CLEAR)
		);

	rttr::registration::enumeration<rhi::AttachmentStoreOperation>("rhi::AttachmentStoreOperation")
		(
			rttr::value("UNDEFINED", rhi::AttachmentStoreOperation::UNDEFINED),
			rttr::value("STORE", rhi::AttachmentStoreOperation::STORE)
		);
}

namespace
{

constexpr std::string_view C_NAME_KEY = "name";
constexpr std::string_view C_SHADER_KEY = "shader";
constexpr std::string_view C_VERSION_KEY = "version";
constexpr std::string_view C_OFFSCREEN_KEY = "offscreen";
constexpr std::string_view C_DEPTH_COMPARE_KEY = "depthCompareOp";
constexpr std::string_view C_CULL_MODE_KEY = "cullMode";
constexpr std::string_view C_COMPUTE_KEY = "compute";
constexpr std::string_view C_ATTACHMENTS_KEY = "attachments";
constexpr std::string_view C_DEPTH_ATTACHMENT_KEY = "depthAttachment";
constexpr std::string_view C_LOAD_OPERATION_KEY = "loadOperation";
constexpr std::string_view C_STORE_OPERATION_KEY = "storeOperation";
constexpr std::string_view C_DEPENDENCY_KEY = "dependency";
constexpr std::string_view C_PATH_KEY = "path";
constexpr std::string_view C_INDEX_KEY = "index";

template<typename T>
T StringToEnum(std::string_view str)
{
	const auto type = rttr::type::get<T>();
	ENGINE_ASSERT(type.is_enumeration());

	const auto enumValue = type.get_enumeration();
	ENGINE_ASSERT(enumValue.is_valid());
	ENGINE_ASSERT(!enumValue.get_type().get_name().empty());

	return enumValue.name_to_value(rttr::string_view(str.data())).template get_value<T>();
}

} // unnamed

namespace engine
{

using namespace nlohmann;

MaterialLoader::MaterialLoader()
{
	m_shaderCompiler = Instance().Service<RenderService>().CreateShaderCompiler();
}

void MaterialLoader::Update()
{
	PROFILER_CPU_ZONE;
}

ResPtr<Resource> MaterialLoader::Load(const fs::path& path)
{
	std::lock_guard l(m_mutex);

	if (auto res = Get(path))
	{
		return res;
	}

	auto resource = MakeResPtr<MaterialResource>(path);
	resource->m_status = Resource::Status::LOADING;
	m_cache[path] = resource;

	auto& ts = Instance().Service<ThreadService>();

	ts.AddBackgroundTask([this, resource]()
		{
			PROFILER_CPU_ZONE_NAME("Load texture");
			const auto result = Load(resource);
			resource->m_status = result ? Resource::Status::READY : Resource::Status::FAILED;
		});

	return resource;
}

ResPtr<Resource> MaterialLoader::Get(const fs::path& path) const
{
	if (const auto it = m_cache.find(path); it != m_cache.end())
	{
		return it->second;
	}
	return {};
}

void MaterialLoader::LoadSystemResources()
{
	m_renderMaterial = std::static_pointer_cast<MaterialResource>(Load("/System/Materials/basic_3d.material"));
	m_skyboxMaterial = std::static_pointer_cast<MaterialResource>(Load("/System/Materials/skybox.material"));
	m_presentMaterial = std::static_pointer_cast<MaterialResource>(Load("/System/Materials/present.material"));
	m_equirectToCubemapMaterial = std::static_pointer_cast<MaterialResource>(Load("/System/Materials/equirect_to_cubemap.material"));
	m_envmapIrradianceMaterial = std::static_pointer_cast<MaterialResource>(Load("/System/Materials/envmap_irradiance.material"));
	m_envmapPrefilterMaterial = std::static_pointer_cast<MaterialResource>(Load("/System/Materials/envmap_prefilter.material"));

	m_renderMaterial->Wait();
	m_presentMaterial->Wait();
	m_skyboxMaterial->Wait();
	m_equirectToCubemapMaterial->Wait();
	m_envmapIrradianceMaterial->Wait();
	m_envmapPrefilterMaterial->Wait();
}

const ResPtr<rhi::Pipeline>& MaterialLoader::Pipeline(const ResPtr<MaterialResource>& res) const
{
	std::lock_guard l(m_mutex);

	ENGINE_ASSERT(res);

	const auto it = m_shaderToPipeline.find(res->Material()->Shader());

	if (it == m_shaderToPipeline.end())
	{
		ENGINE_ASSERT(false);
		static ResPtr<rhi::Pipeline> empty;
		return empty;
	}

	const auto& p = it->second;

	return p;
}

void MaterialLoader::ResizePipelines(glm::ivec2 extent, bool offscreen)
{
	ENGINE_ASSERT(extent != glm::ivec2(0));

	eastl::vector<std::future<void>> tasks;

	for (auto& [_, resource] : m_cache)
	{
		// TODO: Probably we need to resize it later
		if (!resource->Ready())
		{
			continue;
		}

		const auto& pipeline = Pipeline(resource);

		if (pipeline->Descriptor().m_offscreen == offscreen && !pipeline->Descriptor().m_compute)
		{
			auto& ts = Instance().Service<ThreadService>();

			resource->m_status = Resource::Status::LOADING;

			tasks.push_back(ts.AddBackgroundTask([this, resource]()
				{
					PROFILER_CPU_ZONE_NAME("Load texture");
					const auto result = Load(resource, true);
					resource->m_status = result ? Resource::Status::READY : Resource::Status::FAILED;
				}));
		}
	}

	for (auto& task : tasks)
	{
		task.wait();
	}
}

// TODO: Remove compute pass manual texture and storage texture assigning
MaterialLoader::LoadEnvironmentMapData MaterialLoader::LoadEnvironmentMap(const fs::path& path)
{
	auto& rs = Instance().Service<RenderService>();
	auto& resourceService = Instance().Service<ResourceService>();
	const auto envTex = resourceService.Load<TextureResource>(path);
	envTex->Wait();

	LoadEnvironmentMapData data{};

	// Equirect to cubemap
	{
		rhi::TextureDescriptor envCubemapDesc{};
		envCubemapDesc.m_type = rhi::TextureType::TEXTURE_CUBEMAP;
		envCubemapDesc.m_format = rhi::Format::RGBA16_SFLOAT;
		envCubemapDesc.m_layersAmount = 6;
		envCubemapDesc.m_width = 1024;
		envCubemapDesc.m_height = 1024;

		const auto envCubemap = rs.CreateTexture(envCubemapDesc);

		auto& computePass = rs.Pipeline(m_equirectToCubemapMaterial)->Descriptor().m_computePass;
		computePass->m_textures.emplace_back(envTex->Texture());
		computePass->m_storageTextures.emplace_back(envCubemap);

		m_equirectToCubemapMaterial->Material()->SetTexture(envCubemap, 0);
		m_equirectToCubemapMaterial->Material()->SetTexture(envTex->Texture(), 1);
		m_equirectToCubemapMaterial->Material()->Sync();

		const auto state = rs.BeginComputePassImmediate(m_equirectToCubemapMaterial);
		rs.BindMaterial(m_equirectToCubemapMaterial, state);
		rs.Dispatch(envCubemap->Width() / 32, envCubemap->Height() / 32, 6, state);
		rs.EndComputePass(m_equirectToCubemapMaterial, state);

		data.m_cubemap = envCubemap;
	}

	// Compute irradiance
	{
		rhi::TextureDescriptor irrCubemapDesc{};
		irrCubemapDesc.m_type = rhi::TextureType::TEXTURE_CUBEMAP;
		irrCubemapDesc.m_format = rhi::Format::RGBA16_SFLOAT;
		irrCubemapDesc.m_layersAmount = 6;
		irrCubemapDesc.m_width = 32;
		irrCubemapDesc.m_height = 32;

		const auto irrCubemap = rs.CreateTexture(irrCubemapDesc);

		auto& computePass = rs.Pipeline(m_envmapIrradianceMaterial)->Descriptor().m_computePass;
		computePass->m_textures.emplace_back(data.m_cubemap);
		computePass->m_storageTextures.emplace_back(irrCubemap);

		m_envmapIrradianceMaterial->Material()->SetTexture(irrCubemap, 0);
		m_envmapIrradianceMaterial->Material()->SetTexture(data.m_cubemap, 1);
		m_envmapIrradianceMaterial->Material()->Sync();

		const auto state = rs.BeginComputePassImmediate(m_envmapIrradianceMaterial);
		rs.BindMaterial(m_envmapIrradianceMaterial, state);
		rs.Dispatch(irrCubemap->Width() / 32, irrCubemap->Height() / 32, 6, state);
		rs.EndComputePass(m_envmapIrradianceMaterial, state);

		data.m_irradianceTexture = irrCubemap;
	}

	// Compute prefilter map
	{
		rhi::TextureDescriptor prefilterDesc{};
		prefilterDesc.m_type = rhi::TextureType::TEXTURE_CUBEMAP;
		prefilterDesc.m_format = rhi::Format::RGBA16_SFLOAT;
		prefilterDesc.m_layersAmount = 6;
		prefilterDesc.m_mipmapped = true;
		prefilterDesc.m_width = 32;
		prefilterDesc.m_height = 32;
	
		const auto prefilterCubemap = rs.CreateTexture(prefilterDesc);
	
		auto& computePass = rs.Pipeline(m_envmapPrefilterMaterial)->Descriptor().m_computePass;
		computePass->m_textures.emplace_back(data.m_cubemap);
		computePass->m_storageTextures.emplace_back(prefilterCubemap);
	
		for (uint8_t mipLevel = 0; mipLevel < prefilterCubemap->CalculateMipCount(); mipLevel++)
		{
			m_envmapPrefilterMaterial->Material()->SetTexture(prefilterCubemap, 0, mipLevel);
			m_envmapPrefilterMaterial->Material()->SetTexture(data.m_cubemap, 1);
			m_envmapPrefilterMaterial->Material()->Sync();
	
			const auto state = rs.BeginComputePassImmediate(m_envmapPrefilterMaterial);
			rs.BindMaterial(m_envmapPrefilterMaterial, state);
			rs.Dispatch(prefilterCubemap->Width() / 32, prefilterCubemap->Height() / 32, 6, state);
			rs.EndComputePass(m_envmapPrefilterMaterial, state);
		}
	
		data.m_prefilterTexture = prefilterCubemap;
	}

	return data;
}

bool MaterialLoader::Load(const ResPtr<MaterialResource>& resource, bool forcePipelineRecreation)
{
	auto& vfs = Instance().Service<io::VirtualFilesystemService>();

	const auto& srcPath = resource->SourcePath();

	std::ifstream file(vfs.Absolute(srcPath));

	if (file.fail())
	{
		core::log::error("[MaterialLoader] Can't load material '{}'", resource->SourcePath().generic_u8string());
		return false;
	}

	auto parsedMat = ParseJson(file);

	if (parsedMat.m_name.empty())
	{
		return false;
	}

	std::shared_ptr<rhi::Shader> shader;

	{
		std::lock_guard l(m_mutex);
		if (const auto it = m_shaderCache.find(srcPath); it != m_shaderCache.end())
		{
			shader = it->second;
		}
	}

	if (!shader)
	{
		auto& rs = Instance().Service<RenderService>();

		const auto shaderType = parsedMat.m_shaderPath.extension() == ".glsl" ? rhi::ShaderType::FX : rhi::ShaderType::COMPUTE;

		const auto shaderData = rs.RunOnRenderThreadWait([&]()
			{
				return m_shaderCompiler->Compile(vfs.Absolute(parsedMat.m_shaderPath).generic_u8string(), shaderType);
			});

		if (!shaderData.m_valid)
		{
			return false;
		}

		rhi::ShaderDescriptor desc;
		desc.m_name = parsedMat.m_name;
		desc.m_type = shaderType;
		desc.m_reflection = shaderData.m_reflection;
		desc.m_blobByStage = shaderData.m_stageBlob;
		desc.m_path = parsedMat.m_shaderPath.generic_u8string();

		shader = rs.CreateShader(desc);

		parsedMat.m_parsedPipeline.m_shader = shader;

		{
			std::lock_guard l(m_mutex);
			m_shaderCache[parsedMat.m_shaderPath] = shader;
		}
	}

	bool hasPipeline = false;

	{
		std::lock_guard l(m_mutex);
		if (const auto it = m_shaderToPipeline.find(shader); it != m_shaderToPipeline.end())
		{
			hasPipeline = true;
		}
	}

	if (!hasPipeline || forcePipelineRecreation)
	{
		parsedMat.m_parsedPipeline.m_viewportSize = parsedMat.m_parsedPipeline.m_offscreen ? 
			Instance().Service<RenderService>().ViewportSize() :
			Instance().Service<WindowService>().Extent();

		if (parsedMat.m_parsedPipeline.m_viewportSize.x < 1 ||
			parsedMat.m_parsedPipeline.m_viewportSize.y < 1 ||
			parsedMat.m_parsedPipeline.m_viewportSize.x > 65536 ||
			parsedMat.m_parsedPipeline.m_viewportSize.x > 65536)
		{
			core::log::warning("[MaterialLoader] Viewport size is out of bounds: {}x{}", parsedMat.m_parsedPipeline.m_viewportSize.x,
				parsedMat.m_parsedPipeline.m_viewportSize.y);
		}

		glm::clamp(parsedMat.m_parsedPipeline.m_viewportSize, glm::ivec2(1, 1), glm::ivec2(65536, 65536));

		m_shaderToPipeline[shader] = AllocatePipeline(parsedMat.m_parsedPipeline);
	}

	resource->m_material = std::make_shared<render::Material>(shader);

	for (const auto& [slot, buffer] : shader->Descriptor().m_reflection.m_bufferMap)
	{
		rttr::type type = rttr::type::get_by_name(fmt::format("engine::{}", buffer.m_name));
		resource->m_material->SetBuffer(type, slot, buffer.m_stage, buffer.m_name);
	}

	resource->m_material->Sync();
	return true;
}

MaterialLoader::ParsedMaterial MaterialLoader::ParseJson(std::ifstream& stream)
{
	ParsedMaterial mat{};

	auto j = json::parse(stream);

	mat.m_name = j[C_NAME_KEY];
	mat.m_shaderPath = io::fs::path(std::string_view(j[C_SHADER_KEY]));
	mat.m_version = j[C_VERSION_KEY];
	if (!j[C_COMPUTE_KEY].is_null())
	{
		mat.m_parsedPipeline.m_compute = true;
		return mat;
	}
	mat.m_parsedPipeline.m_offscreen = j[C_OFFSCREEN_KEY];
	mat.m_parsedPipeline.m_depthCompareOp = StringToEnum<rhi::CompareOp>(j[C_DEPTH_COMPARE_KEY]);
	mat.m_parsedPipeline.m_cullMode = StringToEnum<rhi::CullMode>(j[C_CULL_MODE_KEY]);

	const auto& attachments = j[C_ATTACHMENTS_KEY];
	ENGINE_ASSERT(attachments.is_array());

	for (auto& attachmentJson : attachments)
	{
		auto& attachment = mat.m_parsedPipeline.m_attachments.emplace_back();
		attachment.m_descriptor.m_clearValue = {};
		attachment.m_descriptor.m_loadOperation = StringToEnum<rhi::AttachmentLoadOperation>(attachmentJson[C_LOAD_OPERATION_KEY]);
		attachment.m_descriptor.m_storeOperation = StringToEnum<rhi::AttachmentStoreOperation>(attachmentJson[C_STORE_OPERATION_KEY]);

		if (attachmentJson.contains(C_DEPENDENCY_KEY))
		{
			auto& dep = attachmentJson[C_DEPENDENCY_KEY];
			attachment.m_dependency = io::fs::path(std::string_view(dep[C_PATH_KEY]));
			attachment.m_depAttachmentIndex = dep[C_INDEX_KEY];
		}
	}

	const auto& parsedDepthAttachment = j[C_DEPTH_ATTACHMENT_KEY];
	if (!parsedDepthAttachment.is_null() && parsedDepthAttachment.is_object())
	{
		LoadAttachmentDescriptor depthAttachment;
		depthAttachment.m_descriptor.m_clearValue = {};
		depthAttachment.m_descriptor.m_loadOperation = StringToEnum<rhi::AttachmentLoadOperation>(parsedDepthAttachment[C_LOAD_OPERATION_KEY]);
		depthAttachment.m_descriptor.m_storeOperation = StringToEnum<rhi::AttachmentStoreOperation>(parsedDepthAttachment[C_STORE_OPERATION_KEY]);

		if (parsedDepthAttachment.contains(C_DEPENDENCY_KEY))
		{
			auto& dep = parsedDepthAttachment[C_DEPENDENCY_KEY];
			depthAttachment.m_dependency = io::fs::path(std::string_view(dep[C_PATH_KEY]));
			depthAttachment.m_depAttachmentIndex = dep[C_INDEX_KEY];
		}

		mat.m_parsedPipeline.m_depthAttachment = depthAttachment;
	}

	ENGINE_ASSERT(mat.m_version != std::numeric_limits<uint8_t>::max());

	return mat;
	
}

std::shared_ptr<rhi::Pipeline> MaterialLoader::AllocatePipeline(ParsedPipelineInfo& info)
{
	auto& rs = Instance().Service<RenderService>();

	if (info.m_compute)
	{
		const auto computePass = std::make_shared<rhi::ComputePass>();

		rhi::PipelineDescriptor computePipelineDesc{};
		computePipelineDesc.m_compute = true;
		computePipelineDesc.m_computePass = computePass;
		computePipelineDesc.m_shader = info.m_shader;

		return rs.CreatePipeline(computePipelineDesc);
	}

	ENGINE_ASSERT(info.m_viewportSize != glm::ivec2(0));

	eastl::vector<rhi::AttachmentDescriptor> colorAttachments;

	for (auto& attachment : info.m_attachments)
	{
		if (attachment.m_dependency.empty())
		{
			rhi::TextureDescriptor colorAttachmentDesc;
			colorAttachmentDesc.m_type = rhi::TextureType::TEXTURE_2D;
			colorAttachmentDesc.m_format = rhi::Format::BGRA8_UNORM;
			colorAttachmentDesc.m_width = static_cast<uint16_t>(info.m_viewportSize.x);
			colorAttachmentDesc.m_height = static_cast<uint16_t>(info.m_viewportSize.y);
			colorAttachmentDesc.m_layersAmount = 1;

			attachment.m_descriptor.m_texture = rs.CreateTexture(colorAttachmentDesc);
		}
		else
		{
			// TODO: Improve logging by adding info for the waiting object
			core::log::debug("[MaterialLoader] Waiting for dependency: '{}'", attachment.m_dependency.generic_u8string());
			auto dependency = std::static_pointer_cast<MaterialResource>(Load(attachment.m_dependency));

			dependency->Wait();

			auto depPipeline = rs.Pipeline(dependency);

			attachment.m_descriptor.m_texture = depPipeline->Descriptor().m_pass->Descriptor().m_colorAttachments[attachment.m_depAttachmentIndex].m_texture;
		}
	}

	if (info.m_depthAttachment)
	{
		auto& depth = *info.m_depthAttachment;
		if (depth.m_dependency.empty())
		{
			rhi::TextureDescriptor depthDesc;
			depthDesc.m_type = rhi::TextureType::TEXTURE_2D;
			depthDesc.m_format = rhi::Format::D32_SFLOAT_S8_UINT;
			depthDesc.m_width = static_cast<uint16_t>(info.m_viewportSize.x);
			depthDesc.m_height = static_cast<uint16_t>(info.m_viewportSize.y);
			depthDesc.m_layersAmount = 1;

			depth.m_descriptor.m_texture = rs.CreateTexture(depthDesc);
		}
		else
		{
			// TODO: Improve logging by adding info for the waiting object
			core::log::debug("[MaterialLoader] Waiting for dependency: '{}'", depth.m_dependency.generic_u8string());
			auto dependency = std::static_pointer_cast<MaterialResource>(Load(depth.m_dependency));

			dependency->Wait();

			auto depPipeline = rs.Pipeline(dependency);

			depth.m_descriptor.m_texture = depPipeline->Descriptor().m_pass->Descriptor().m_depthStencilAttachment.m_texture;
		}
	}

	rhi::RenderPassDescriptor renderPassDesc{};
	renderPassDesc.m_name = fmt::format("{}-Pass", info.m_shader->Descriptor().m_name);
	renderPassDesc.m_extent = info.m_viewportSize;

	for (auto& attachment : info.m_attachments)
	{
		renderPassDesc.m_colorAttachments.emplace_back(std::move(attachment.m_descriptor));
	}

	renderPassDesc.m_depthStencilAttachment = info.m_depthAttachment.has_value() ? info.m_depthAttachment->m_descriptor : rhi::AttachmentDescriptor{};

	const auto renderpass = rs.CreateRenderPass(renderPassDesc);

	rhi::PipelineDescriptor pipelineDesc{};
	pipelineDesc.m_compute = false;
	pipelineDesc.m_cullMode = info.m_cullMode;
	pipelineDesc.m_depthCompareOp = info.m_depthCompareOp;
	pipelineDesc.m_offscreen = info.m_offscreen;
	pipelineDesc.m_pass = renderpass;
	pipelineDesc.m_shader = info.m_shader;

	const auto pipeline = rs.CreatePipeline(pipelineDesc);
	return pipeline;
}

MaterialResource::MaterialResource(const io::fs::path& path) : Resource(path)
{
}

} // engine