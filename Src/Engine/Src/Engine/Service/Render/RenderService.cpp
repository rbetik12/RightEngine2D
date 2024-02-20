#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Engine.hpp>
#include <RHI/Pipeline.hpp>

#pragma warning(push)
#pragma warning(disable : 4464)
#include <glslang/Public/ShaderLang.h>
#pragma warning(pop)

RTTR_REGISTRATION
{
engine::registration::Service<engine::RenderService>("engine::RenderService")
    .Domain(engine::Domain::UI);
}

namespace
{

const eastl::vector<float> vertexBufferRaw =
{
	-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
};

const eastl::vector<float> presentVBRaw =
{
	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

	1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};

} // unnamed

namespace engine
{

struct RenderService::Impl
{
    std::shared_ptr<rhi::ShaderCompiler>	m_shaderCompiler;
    std::shared_ptr<rhi::Shader>			m_shader;
	std::shared_ptr<rhi::Shader>			m_presentShader;
    std::shared_ptr<rhi::Buffer>			m_buffer;
    std::shared_ptr<rhi::Buffer>			m_presentVB;
    std::shared_ptr<rhi::Texture>			m_texture;
    std::shared_ptr<rhi::RenderPass>		m_renderPass;
    std::shared_ptr<rhi::Pipeline>			m_pipeline;
    std::shared_ptr<rhi::Pipeline>			m_presentPipeline;
    std::unique_ptr<render::Material>		m_presentMaterial;

	std::shared_ptr<rhi::RenderPass>		m_imguiRenderPass;
};

RenderService::RenderService()
{
	m_impl = std::make_unique<Impl>();

	rhi::vulkan::VulkanInitContext initCtx;
    initCtx.m_surfaceConstructor = [&](VkInstance instance)
    {
        VkSurfaceKHR surface = nullptr;
        glfwCreateWindowSurface(instance, Instance().Service<WindowService>().Window(), nullptr, &surface);
        return surface;
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    eastl::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef R_APPLE
    extensions.push_back("VK_KHR_portability_enumeration");
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif
    initCtx.m_requiredExtensions = std::move(extensions);

    m_context = rhi::vulkan::CreateContext(std::move(initCtx));
    m_device = rhi::Device::Create(m_context);

	m_defaultSampler = m_device->CreateSampler({});
	LoadSystemResources();

    const auto extent = Instance().Service<WindowService>().Extent();
	OnResize(extent.x, extent.y);
}

RenderService::~RenderService()
{
	m_device->WaitForIdle();
	m_impl.reset();
    m_defaultSampler.reset();
    m_device.reset();
    m_context.reset();
}

void RenderService::Update(float dt)
{
	PROFILER_CPU_ZONE;
    m_device->BeginFrame();

	BeginPass(m_impl->m_pipeline);
	Draw(m_impl->m_buffer, m_impl->m_buffer->Descriptor().m_size /
		m_impl->m_pipeline->Descriptor().m_shader->Descriptor().m_reflection.m_inputLayout.Stride());
	EndPass(m_impl->m_pipeline);
}

void RenderService::PostUpdate(float dt)
{
	PROFILER_CPU_ZONE;
	BeginPass(m_impl->m_presentPipeline);
	Draw(m_impl->m_presentVB, m_impl->m_presentVB->Descriptor().m_size /
		m_impl->m_presentPipeline->Descriptor().m_shader->Descriptor().m_reflection.m_inputLayout.Stride());
	EndPass(m_impl->m_presentPipeline);

    m_device->EndFrame();
    m_device->Present();
}

std::shared_ptr<rhi::ShaderCompiler> RenderService::CreateShaderCompiler(const rhi::ShaderCompiler::Options& options)
{
    return m_device->CreateShaderCompiler(options);
}

std::shared_ptr<rhi::Buffer> RenderService::CreateBuffer(const rhi::BufferDescriptor& desc, const void* data)
{
    return m_device->CreateBuffer(desc, data);
}

std::shared_ptr<rhi::Texture> RenderService::CreateTexture(const rhi::TextureDescriptor& desc, const std::shared_ptr<rhi::Sampler>& sampler, const void* data)
{
    if (sampler)
    {
        return m_device->CreateTexture(desc, sampler, data);
    }
    return m_device->CreateTexture(desc, m_defaultSampler, data);
}

std::shared_ptr<rhi::Shader> RenderService::CreateShader(const rhi::ShaderDescriptor& desc)
{
    return m_device->CreateShader(desc);
}

std::shared_ptr<rhi::Sampler> RenderService::CreateSampler(const rhi::SamplerDescriptor& desc)
{
    return m_device->CreateSampler(desc);
}

std::shared_ptr<rhi::RenderPass> RenderService::CreateRenderPass(const rhi::RenderPassDescriptor& desc)
{
    return m_device->CreateRenderPass(desc);
}

std::shared_ptr<rhi::Pipeline> RenderService::CreatePipeline(const rhi::PipelineDescriptor& desc)
{
    return m_device->CreatePipeline(desc);
}

void RenderService::BeginPass(const std::shared_ptr<rhi::Pipeline>& pipeline)
{
    m_device->BeginPipeline(pipeline);
}

void RenderService::EndPass(const std::shared_ptr<rhi::Pipeline>& pipeline)
{
    m_device->EndPipeline(pipeline);
}

void RenderService::Draw(const std::shared_ptr<rhi::Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount)
{
    m_device->Draw(buffer, vertexCount, instanceCount);
}

void RenderService::WaitAll()
{
    m_device->WaitForIdle();
}

void RenderService::OnResize(uint32_t weight, uint32_t height)
{
	PROFILER_CPU_ZONE;
    m_device->OnResize(weight, height);
	CreateRenderResources(weight, height);
}

const std::shared_ptr<rhi::RenderPass>& RenderService::ImGuiPass() const
{
	return m_impl->m_imguiRenderPass;
}

void RenderService::CreateRenderResources(uint32_t width, uint32_t height)
{
	PROFILER_CPU_ZONE;
	rhi::TextureDescriptor textureDescriptor;
	textureDescriptor.m_width = static_cast<uint16_t>(width);
	textureDescriptor.m_height = static_cast<uint16_t>(height);
	textureDescriptor.m_componentAmount = 4;
	textureDescriptor.m_type = rhi::TextureType::TEXTURE_2D;
	textureDescriptor.m_layersAmount = 1;
	textureDescriptor.m_format = rhi::Format::BGRA8_UNORM;

	m_impl->m_texture = CreateTexture(textureDescriptor);

	rhi::RenderPassDescriptor renderPassDescriptor;
	renderPassDescriptor.m_extent = { width, height };
	renderPassDescriptor.m_name = "Triangle";
	renderPassDescriptor.m_colorAttachments = { {m_impl->m_texture} };

	m_impl->m_renderPass = CreateRenderPass(renderPassDescriptor);

	rhi::PipelineDescriptor pipelineDescriptor{};
	pipelineDescriptor.m_pass = m_impl->m_renderPass;
	pipelineDescriptor.m_shader = m_impl->m_shader;
	pipelineDescriptor.m_cullMode = rhi::CullMode::NONE;

	m_impl->m_pipeline = CreatePipeline(pipelineDescriptor);

	rhi::RenderPassDescriptor presentRenderpassDescriptor{};
	presentRenderpassDescriptor.m_extent = { width, height };
	presentRenderpassDescriptor.m_name = "Present";

	const auto presentRenderpass = CreateRenderPass(presentRenderpassDescriptor);

	rhi::PipelineDescriptor presentPipelineDescritor{};
	presentPipelineDescritor.m_cullMode = rhi::CullMode::NONE;
	presentPipelineDescritor.m_offscreen = false;
	presentPipelineDescritor.m_pass = presentRenderpass;
	presentPipelineDescritor.m_shader = m_impl->m_presentShader;

	m_impl->m_presentPipeline = CreatePipeline(presentPipelineDescritor);

	if (!m_impl->m_presentMaterial)
	{
		m_impl->m_presentMaterial = std::make_unique<render::Material>(m_impl->m_presentShader);
	}

	m_impl->m_presentMaterial->SetTexture(m_impl->m_texture, 0);
	m_impl->m_presentMaterial->Sync();

	{
		rhi::RenderPassDescriptor imguiPassDesc{};
		imguiPassDesc.m_extent = { width, height };
		imguiPassDesc.m_name = "ImGui";

		rhi::AttachmentDescriptor desc{};
		desc.m_texture = m_impl->m_texture;
		desc.m_loadOperation = rhi::AttachmentLoadOperation::LOAD;

		imguiPassDesc.m_colorAttachments.emplace_back(desc);

		m_impl->m_imguiRenderPass = CreateRenderPass(imguiPassDesc);
	}
}

void RenderService::LoadSystemResources()
{
	auto& vfs = Instance().Service<io::VirtualFilesystemService>();

	m_impl->m_shaderCompiler = CreateShaderCompiler();

	const auto shaderPath = "/System/Shaders/basic.glsl";
	const auto compiledShader = m_impl->m_shaderCompiler->Compile(vfs.Absolute(io::fs::path(shaderPath)).generic_u8string());

	rhi::ShaderDescriptor shaderDescriptor;
	shaderDescriptor.m_path = shaderPath;
	shaderDescriptor.m_name = "Basic";
	shaderDescriptor.m_blobByStage = compiledShader.m_stageBlob;
	shaderDescriptor.m_reflection = compiledShader.m_reflection;
	shaderDescriptor.m_type = rhi::ShaderType::FX;

	m_impl->m_shader = CreateShader(shaderDescriptor);

	const auto presentShaderPath = "/System/Shaders/present.glsl";
	const auto presentShaderData = m_impl->m_shaderCompiler->Compile(vfs.Absolute(io::fs::path(presentShaderPath)).generic_u8string());

	rhi::ShaderDescriptor presentShaderDesc{};
	presentShaderDesc.m_path = presentShaderPath;
	presentShaderDesc.m_blobByStage = presentShaderData.m_stageBlob;
	presentShaderDesc.m_name = "Present";
	presentShaderDesc.m_type = rhi::ShaderType::FX;
	presentShaderDesc.m_reflection = presentShaderData.m_reflection;

	m_impl->m_presentShader = CreateShader(presentShaderDesc);

	rhi::BufferDescriptor bufferDesc{};
	bufferDesc.m_memoryType = rhi::MemoryType::CPU_GPU;
	bufferDesc.m_type = rhi::BufferType::VERTEX;
	bufferDesc.m_size = sizeof(vertexBufferRaw[0]) * static_cast<uint32_t>(vertexBufferRaw.size());

	m_impl->m_buffer = CreateBuffer(bufferDesc, vertexBufferRaw.data());

	rhi::BufferDescriptor presentVBDesc{};
	presentVBDesc.m_size = sizeof(presentVBRaw[0]) * static_cast<uint32_t>(presentVBRaw.size());
	presentVBDesc.m_memoryType = rhi::MemoryType::CPU_GPU;
	presentVBDesc.m_name = "PresentVB";
	presentVBDesc.m_type = rhi::BufferType::VERTEX;

	m_impl->m_presentVB = CreateBuffer(presentVBDesc, presentVBRaw.data());
}

} // engine
