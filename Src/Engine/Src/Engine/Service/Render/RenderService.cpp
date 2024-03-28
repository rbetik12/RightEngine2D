#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <Engine/Service/Window/WindowService.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Engine.hpp>
#include <RHI/Pipeline.hpp>

#include "Engine/Service/Imgui/ImguiService.hpp"

RTTR_REGISTRATION
{
engine::registration::Service<engine::RenderService>("engine::RenderService")
    .Domain(engine::Domain::UI);
}

namespace
{

const eastl::vector<float> presentVBRaw =
{
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};

constexpr uint32_t C_MAX_RESOLUTION = 65536;

} // unnamed

namespace engine
{

struct RenderService::Impl
{
    std::shared_ptr<rhi::ShaderCompiler>    m_shaderCompiler;
    std::shared_ptr<rhi::Buffer>            m_presentVB;
    std::shared_ptr<rhi::Texture>           m_texture;
    std::shared_ptr<rhi::RenderPass>        m_renderPass;
    std::shared_ptr<rhi::Pipeline>          m_pipeline;
    std::shared_ptr<rhi::Pipeline>          m_presentPipeline;

    std::shared_ptr<rhi::Shader>            m_defaultShader;
    std::unique_ptr<render::Material>       m_presentMaterial;

    std::shared_ptr<rhi::RenderPass>        m_imguiRenderPass;

    glm::ivec2                              m_newResolution;
    bool                                    m_resizeRequested = true;
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

    auto extent = Instance().Service<WindowService>().Extent();
    OnWindowResize(extent.x, extent.y);
    m_impl->m_newResolution = std::move(extent);
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

    if (m_impl->m_resizeRequested)
    {
        PROFILER_CPU_ZONE_NAME("Resize render resources");

        ENGINE_ASSERT(m_impl->m_newResolution != glm::ivec2());
        m_device->WaitForIdle();
        CreateRenderResources(m_impl->m_newResolution.x, m_impl->m_newResolution.y);
        m_impl->m_resizeRequested = false;
    }

    m_device->BeginFrame();
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

RPtr<rhi::ShaderCompiler> RenderService::CreateShaderCompiler(const rhi::ShaderCompiler::Options& options)
{
    return m_device->CreateShaderCompiler(options);
}

RPtr<rhi::Buffer> RenderService::CreateBuffer(const rhi::BufferDescriptor& desc, const void* data)
{
    ENGINE_ASSERT(!desc.m_name.empty());

    return m_device->CreateBuffer(desc, data);
}

RPtr<rhi::Texture> RenderService::CreateTexture(const rhi::TextureDescriptor& desc, const std::shared_ptr<rhi::Sampler>& sampler, const void* data)
{
    if (sampler)
    {
        return m_device->CreateTexture(desc, sampler, data);
    }
    return m_device->CreateTexture(desc, m_defaultSampler, data);
}

RPtr<rhi::Shader> RenderService::CreateShader(const rhi::ShaderDescriptor& desc)
{
    ENGINE_ASSERT(!desc.m_name.empty());

    return m_device->CreateShader(desc);
}

RPtr<rhi::Sampler> RenderService::CreateSampler(const rhi::SamplerDescriptor& desc)
{
    return m_device->CreateSampler(desc);
}

RPtr<rhi::RenderPass> RenderService::CreateRenderPass(const rhi::RenderPassDescriptor& desc)
{
    return m_device->CreateRenderPass(desc);
}

RPtr<rhi::Pipeline> RenderService::CreatePipeline(const rhi::PipelineDescriptor& desc)
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

void RenderService::BeginComputePass(const std::shared_ptr<rhi::Pipeline>& pipeline)
{
    m_device->BeginComputePipeline(pipeline);
}

void RenderService::EndComputePass(const std::shared_ptr<rhi::Pipeline>& pipeline)
{
    m_device->BeginComputePipeline(pipeline);
}

void RenderService::Draw(const std::shared_ptr<rhi::Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount)
{
    m_device->Draw(buffer, vertexCount, instanceCount);
}

void RenderService::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    m_device->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void RenderService::WaitAll()
{
    m_device->WaitForIdle();
}

void RenderService::OnResize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0)
    {
        core::log::warning("[RenderService] Resize with zeroed dimension was requested, clamping it to 1: {}x{}", width, height);
    }
    m_impl->m_resizeRequested = true;
    m_impl->m_newResolution = { glm::clamp(width, 1u, C_MAX_RESOLUTION), glm::clamp(height, 1u, C_MAX_RESOLUTION) };
}

void RenderService::OnWindowResize(uint32_t width, uint32_t height)
{
    PROFILER_CPU_ZONE;

    m_device->OnResize(width, height);
    CreateWindowResources(width, height);

    if ((Instance().Cfg().m_domain & Domain::EDITOR) != Domain::EDITOR)
    {
        OnResize(width, height);
    }
}

const RPtr<rhi::RenderPass>& RenderService::BasicPass() const
{
    return m_impl->m_renderPass;
}

const RPtr<rhi::RenderPass>& RenderService::ImGuiPass() const
{
    return m_impl->m_imguiRenderPass;
}

const RPtr<rhi::ShaderCompiler>& RenderService::ShaderCompiler() const
{
    return m_impl->m_shaderCompiler;
}

const RPtr<rhi::Shader>& RenderService::DefaultShader() const
{
    return m_impl->m_defaultShader;
}

const RPtr<rhi::Pipeline>& RenderService::DefaultPipeline() const
{
    return m_impl->m_pipeline;
}

void RenderService::CreateRenderResources(uint32_t width, uint32_t height)
{
    PROFILER_CPU_ZONE;

    if (const auto imguiService = Instance().FindService<ImguiService>(); imguiService && m_impl->m_texture)
    {
        imguiService->RemoveImage(m_impl->m_texture);
    }

    rhi::TextureDescriptor textureDescriptor;
    textureDescriptor.m_width = static_cast<uint16_t>(width);
    textureDescriptor.m_height = static_cast<uint16_t>(height);
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
    pipelineDescriptor.m_shader = m_impl->m_defaultShader;
    pipelineDescriptor.m_cullMode = rhi::CullMode::NONE;

    m_impl->m_pipeline = CreatePipeline(pipelineDescriptor);
}

void RenderService::CreateWindowResources(uint32_t width, uint32_t height)
{
    PROFILER_CPU_ZONE;

    rhi::RenderPassDescriptor presentRenderpassDescriptor{};
    presentRenderpassDescriptor.m_extent = { width, height };
    presentRenderpassDescriptor.m_name = "Present";

    const auto presentRenderpass = CreateRenderPass(presentRenderpassDescriptor);

    rhi::PipelineDescriptor presentPipelineDescritor{};
    presentPipelineDescritor.m_cullMode = rhi::CullMode::NONE;
    presentPipelineDescritor.m_offscreen = false;
    presentPipelineDescritor.m_pass = presentRenderpass;
    presentPipelineDescritor.m_shader = m_impl->m_presentMaterial->Shader();

    m_impl->m_presentPipeline = CreatePipeline(presentPipelineDescritor);

    rhi::TextureDescriptor textureDescriptor;
    textureDescriptor.m_width = static_cast<uint16_t>(width);
    textureDescriptor.m_height = static_cast<uint16_t>(height);
    textureDescriptor.m_type = rhi::TextureType::TEXTURE_2D;
    textureDescriptor.m_layersAmount = 1;
    textureDescriptor.m_format = rhi::Format::BGRA8_UNORM;

    const auto imguiTex = CreateTexture(textureDescriptor);

    // TODO: Add here rendering from PBR pass to swapchain in non-editor mode
    m_impl->m_presentMaterial->SetTexture(imguiTex, 0);
    m_impl->m_presentMaterial->Sync();

    rhi::RenderPassDescriptor imguiPassDesc{};
    imguiPassDesc.m_extent = { width, height };
    imguiPassDesc.m_name = "ImGui";

    rhi::AttachmentDescriptor desc{};
    desc.m_texture = imguiTex;

    imguiPassDesc.m_colorAttachments.emplace_back(desc);

    m_impl->m_imguiRenderPass = CreateRenderPass(imguiPassDesc);
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

    m_impl->m_defaultShader = CreateShader(shaderDescriptor);

    const auto presentShaderPath = "/System/Shaders/present.glsl";
    const auto presentShaderData = m_impl->m_shaderCompiler->Compile(vfs.Absolute(io::fs::path(presentShaderPath)).generic_u8string());

    rhi::ShaderDescriptor presentShaderDesc{};
    presentShaderDesc.m_path = presentShaderPath;
    presentShaderDesc.m_blobByStage = presentShaderData.m_stageBlob;
    presentShaderDesc.m_name = "Present";
    presentShaderDesc.m_type = rhi::ShaderType::FX;
    presentShaderDesc.m_reflection = presentShaderData.m_reflection;

    const auto presentShader = CreateShader(presentShaderDesc);

    rhi::BufferDescriptor presentVBDesc{};
    presentVBDesc.m_size = sizeof(presentVBRaw[0]) * static_cast<uint32_t>(presentVBRaw.size());
    presentVBDesc.m_memoryType = rhi::MemoryType::CPU_GPU;
    presentVBDesc.m_name = "PresentVB";
    presentVBDesc.m_type = rhi::BufferType::VERTEX;

    m_impl->m_presentVB = CreateBuffer(presentVBDesc, presentVBRaw.data());

    m_impl->m_presentMaterial = std::make_unique<render::Material>(presentShader);
}

} // engine
