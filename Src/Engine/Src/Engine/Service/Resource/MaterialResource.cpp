#include <Engine/Service/Resource/MaterialResource.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/Filesystem/File.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <RHI/Helpers.hpp>
#include <nlohmann/json.hpp>

#include "RHI/Pipeline.hpp"
#include "RHI/RenderPass.hpp"

RTTR_REGISTRATION
{
	engine::registration::Class<engine::MaterialLoader>("engine::MaterialLoader");

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
constexpr std::string_view C_ATTACHMENTS_KEY = "attachments";
constexpr std::string_view C_DEPTH_ATTACHMENT_KEY = "depthAttachment";
constexpr std::string_view C_LOAD_OPERATION_KEY = "loadOperation";
constexpr std::string_view C_STORE_OPERATION_KEY = "storeOperation";

template<typename T>
T StringToEnum(std::string_view str)
{
	const auto type = rttr::type::get<T>();
	ENGINE_ASSERT(type.is_enumeration());

	const auto enumValue = type.get_enumeration();
	ENGINE_ASSERT(enumValue.is_valid());

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
	auto renderMat = Load("/System/Materials/basic_3d.material");
	auto presentMat = Load("/System/Materials/present.material");

	renderMat->Wait();
	presentMat->Wait();
}

const ResPtr<rhi::Pipeline>& MaterialLoader::Pipeline(const ResPtr<MaterialResource>& res) const
{
	ENGINE_ASSERT(res);

	const auto it = m_shaderToPipeline.find(res->Material()->Shader());

	if (it == m_shaderToPipeline.end())
	{
		ENGINE_ASSERT(false);
		static ResPtr<rhi::Pipeline> empty;
		return empty;
	}

	return it->second;
}

void MaterialLoader::ResizePipelines(glm::ivec2 extent, bool onScreen)
{
	ENGINE_ASSERT(extent != glm::ivec2(0));

	for (auto& it : m_shaderToPipeline)
	{
		auto& oldPipelineDesc = it.second->Descriptor();

	    if (oldPipelineDesc.m_offscreen == onScreen)
	    {
			ParsedPipelineInfo pipelineInfo{};
			pipelineInfo.m_viewportSize = extent;
			pipelineInfo.m_shader = it.first;
			pipelineInfo.m_offscreen = onScreen;
			pipelineInfo.m_attachments = oldPipelineDesc.m_pass->Descriptor().m_colorAttachments;
			pipelineInfo.m_depthAttachment = oldPipelineDesc.m_pass->Descriptor().m_depthStencilAttachment;
			pipelineInfo.m_compute = oldPipelineDesc.m_compute;
			pipelineInfo.m_cullMode = oldPipelineDesc.m_cullMode;
			pipelineInfo.m_depthCompareOp = oldPipelineDesc.m_depthCompareOp;

			it.second = AllocatePipeline(pipelineInfo);
	    }
	}
}

bool MaterialLoader::Load(const ResPtr<MaterialResource>& resource)
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
		const auto shaderData = m_shaderCompiler->Compile(vfs.Absolute(parsedMat.m_shaderPath).generic_u8string(), shaderType);

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

	if (!hasPipeline)
	{
		parsedMat.m_parsedPipeline.m_viewportSize = Instance().Service<RenderService>().ViewportSize();

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
	mat.m_parsedPipeline.m_offscreen = j[C_OFFSCREEN_KEY];
	mat.m_parsedPipeline.m_depthCompareOp = StringToEnum<rhi::CompareOp>(j[C_DEPTH_COMPARE_KEY]);
	mat.m_parsedPipeline.m_cullMode = StringToEnum<rhi::CullMode>(j[C_CULL_MODE_KEY]);

	const auto& attachments = j[C_ATTACHMENTS_KEY];
	ENGINE_ASSERT(attachments.is_array());

	for (auto& attachmentJson : attachments)
	{
		auto& attachment = mat.m_parsedPipeline.m_attachments.emplace_back();
		attachment.m_clearValue = {};
		attachment.m_loadOperation = StringToEnum<rhi::AttachmentLoadOperation>(attachmentJson[C_LOAD_OPERATION_KEY]);
		attachment.m_storeOperation = StringToEnum<rhi::AttachmentStoreOperation>(attachmentJson[C_STORE_OPERATION_KEY]);
	}

	const auto& parsedDepthAttachment = j[C_DEPTH_ATTACHMENT_KEY];
	if (!parsedDepthAttachment.is_null() && parsedDepthAttachment.is_object())
	{
		auto& depthAttachment = mat.m_parsedPipeline.m_depthAttachment;
		depthAttachment.m_clearValue = {};
		depthAttachment.m_loadOperation = StringToEnum<rhi::AttachmentLoadOperation>(parsedDepthAttachment[C_LOAD_OPERATION_KEY]);
		depthAttachment.m_storeOperation = StringToEnum<rhi::AttachmentStoreOperation>(parsedDepthAttachment[C_STORE_OPERATION_KEY]);
	}

	ENGINE_ASSERT(mat.m_version != std::numeric_limits<uint8_t>::max());

	return mat;
	
}

std::shared_ptr<rhi::Pipeline> MaterialLoader::AllocatePipeline(ParsedPipelineInfo& info)
{
	ENGINE_ASSERT(info.m_viewportSize != glm::ivec2(0));

	eastl::vector<rhi::AttachmentDescriptor> colorAttachments;

	rhi::TextureDescriptor colorAttachmentDesc;
	colorAttachmentDesc.m_type = rhi::TextureType::TEXTURE_2D;
	colorAttachmentDesc.m_format = rhi::Format::RGBA16_SFLOAT;
	colorAttachmentDesc.m_width = static_cast<uint16_t>(info.m_viewportSize.x);
	colorAttachmentDesc.m_height = static_cast<uint16_t>(info.m_viewportSize.y);
	colorAttachmentDesc.m_layersAmount = 1;
	colorAttachmentDesc.m_mipLevels = 1;

	auto& rs = Instance().Service<RenderService>();

	for (auto& attachment : info.m_attachments)
	{
		attachment.m_texture = rs.CreateTexture(colorAttachmentDesc);
	}

	rhi::RenderPassDescriptor renderPassDesc{};
	renderPassDesc.m_name = fmt::format("{}-Pass", info.m_shader->Descriptor().m_name);
	renderPassDesc.m_extent = info.m_viewportSize;
	renderPassDesc.m_colorAttachments = std::move(info.m_attachments);
	renderPassDesc.m_depthStencilAttachment = {};

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