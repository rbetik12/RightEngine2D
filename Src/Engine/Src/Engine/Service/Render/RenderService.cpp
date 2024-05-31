#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <Engine/Service/Window/WindowService.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Service/EditorService.hpp>
#include <Engine/System/RenderSystem.hpp>
#include <Engine/Engine.hpp>
#include <RHI/Pipeline.hpp>
#include <RHI/GPUMaterial.hpp>

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

inline engine::MaterialLoader& GetMaterialLoader()
{
    return engine::Instance().Service<engine::ResourceService>().GetLoader<engine::MaterialLoader>();
}

} // unnamed

namespace engine
{

struct RenderService::Impl
{
    std::shared_ptr<rhi::Device>            m_device;
    std::shared_ptr<rhi::Sampler>           m_defaultSampler;
    std::shared_ptr<rhi::IContext>          m_context;

    std::shared_ptr<rhi::Buffer>            m_presentVB;
    std::shared_ptr<rhi::Texture>           m_texture;

    std::shared_ptr<MaterialResource>       m_renderMaterial;
    std::shared_ptr<MaterialResource>       m_presentMaterial;

    std::shared_ptr<rhi::RenderPass>        m_imguiRenderPass;

    std::mutex                              m_pipelineGetterMutex;

    glm::ivec2                              m_newResolution;
    bool                                    m_resizeRequested = true;
};

RenderService::RenderService()
{
    m_impl = std::make_unique<Impl>();

    m_renderThread = Instance().Service<ThreadService>().SpawnThread("Render Thread");

    RunOnRenderThreadWait([]()
        {
            core::ThreadIdStorage::SetRenderThreadId(core::CurrentThreadID());
        });

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

    m_impl->m_context = rhi::vulkan::CreateContext(std::move(initCtx));
    m_impl->m_device = rhi::Device::Create(m_impl->m_context);

    m_impl->m_defaultSampler = m_impl->m_device->CreateSampler({});
}

RenderService::~RenderService()
{
    m_impl->m_device->WaitForIdle();
    m_impl->m_defaultSampler.reset();
    m_impl->m_device.reset();
    m_impl->m_context.reset();
    m_impl.reset();
}

void RenderService::Update(float dt)
{
    PROFILER_CPU_ZONE;

    if (m_impl->m_resizeRequested)
    {
        PROFILER_CPU_ZONE_NAME("Resize render resources");

        ENGINE_ASSERT(m_impl->m_newResolution != glm::ivec2());
        m_impl->m_device->WaitForIdle();
        CreateRenderResources(m_impl->m_newResolution);
        m_impl->m_resizeRequested = false;
    }

    m_impl->m_device->BeginFrame();
}

void RenderService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;

    BeginPass(m_impl->m_presentMaterial);
    BindMaterial(m_impl->m_presentMaterial);
    
    Draw(m_impl->m_presentVB, m_impl->m_presentVB->Descriptor().m_size /
        Pipeline(m_impl->m_presentMaterial)->Descriptor().m_shader->Descriptor().m_reflection.m_inputLayout.Stride());
    
    EndPass(m_impl->m_presentMaterial);

    RunOnRenderThread([=]
        {
            m_impl->m_device->EndFrame();
            m_impl->m_device->Present();
        });

    {
        PROFILER_CPU_ZONE_NAME("Wait for render thread");
        m_renderThread->WaitForAll();
    }
}

RPtr<rhi::ShaderCompiler> RenderService::CreateShaderCompiler(const rhi::ShaderCompiler::Options& options)
{
    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreateShaderCompiler(options);
        });
}

RPtr<rhi::Buffer> RenderService::CreateBuffer(const rhi::BufferDescriptor& desc, const void* data)
{
    ENGINE_ASSERT(!desc.m_name.empty());

    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreateBuffer(desc, data);
        });
}

RPtr<rhi::Texture> RenderService::CreateTexture(const rhi::TextureDescriptor& desc, const std::shared_ptr<rhi::Sampler>& sampler, const void* data)
{
    if (sampler)
    {
        return RunOnRenderThreadWait([&]()
            {
                return m_impl->m_device->CreateTexture(desc, sampler, data);
            });
    }
    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreateTexture(desc, m_impl->m_defaultSampler, data);
        });
}

RPtr<rhi::Shader> RenderService::CreateShader(const rhi::ShaderDescriptor& desc)
{
    ENGINE_ASSERT(!desc.m_name.empty());

    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreateShader(desc);
        });
}

RPtr<rhi::Sampler> RenderService::CreateSampler(const rhi::SamplerDescriptor& desc)
{
    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreateSampler(desc);
        });
}

RPtr<rhi::RenderPass> RenderService::CreateRenderPass(const rhi::RenderPassDescriptor& desc)
{
    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreateRenderPass(desc);
        });
}

RPtr<rhi::Pipeline> RenderService::CreatePipeline(const rhi::PipelineDescriptor& desc)
{
    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreatePipeline(desc);
        });
}

RPtr<rhi::GPUMaterial> RenderService::CreateGPUMaterial(const std::shared_ptr<rhi::Shader>& shader)
{
    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->CreateGPUMaterial(shader);
        });
}

void RenderService::BeginPass(const ResPtr<MaterialResource>& material)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->BeginPipeline(Pipeline(material));
        });
}

void RenderService::BeginPass(const std::shared_ptr<rhi::Pipeline>& pipeline)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->BeginPipeline(pipeline);
        });
}

void RenderService::EndPass(const ResPtr<MaterialResource>& material)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->EndPipeline(Pipeline(material));
        });
}

void RenderService::EndPass(const std::shared_ptr<rhi::Pipeline>& pipeline)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->EndPipeline(pipeline);
        });
}

void RenderService::BeginComputePass(const ResPtr<MaterialResource>& material)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->BeginComputePipeline(Pipeline(material));
        });
}

RPtr<rhi::ComputeState> RenderService::BeginComputePassImmediate(const ResPtr<MaterialResource>& material)
{
    return RunOnRenderThreadWait([&]()
        {
            return m_impl->m_device->BeginComputePipelineImmediate(Pipeline(material));
        });
}

void RenderService::EndComputePass(const ResPtr<MaterialResource>& material)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->EndComputePipeline(Pipeline(material));
        });
}

void RenderService::EndComputePass(const ResPtr<MaterialResource>& material, const RPtr<rhi::ComputeState>& state)
{
    RunOnRenderThreadWait([&]()
        {
            m_impl->m_device->EndComputePipeline(Pipeline(material), state);
        });
}

void RenderService::Draw(const std::shared_ptr<rhi::Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->Draw(buffer, vertexCount, instanceCount);
        });
}

void RenderService::Draw(const std::shared_ptr<rhi::Buffer>& vb, const std::shared_ptr<rhi::Buffer>& ib, uint32_t instanceCount)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->Draw(vb, ib, ib->Descriptor().m_size / sizeof(uint32_t), instanceCount);
        });
}

void RenderService::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->Dispatch(groupCountX, groupCountY, groupCountZ);
        });
}

void RenderService::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
    const RPtr<rhi::ComputeState>& state)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->Dispatch(groupCountX, groupCountY, groupCountZ, state);
        });
}

void RenderService::BindMaterial(const ResPtr<MaterialResource>& material)
{
    auto& pipeline = Pipeline(material);

    RunOnRenderThread([=]()
        {
            if (const auto gpuMaterial = material->Material()->GPUMaterial())
            {
                m_impl->m_device->BindGPUMaterial(material->Material()->GPUMaterial(), pipeline);
            }
        });
}

void RenderService::BindMaterial(const ResPtr<MaterialResource>& material, const RPtr<rhi::ComputeState>& state)
{
    RunOnRenderThread([=]()
        {
            if (const auto gpuMaterial = material->Material()->GPUMaterial())
            {
                m_impl->m_device->BindGPUMaterial(material->Material()->GPUMaterial(), Pipeline(material), state);
            }
        });
}

void RenderService::PushConstant(const void* data, uint32_t size, const std::shared_ptr<rhi::Pipeline>& pipeline)
{
    RunOnRenderThread([=]()
        {
            m_impl->m_device->PushConstant(data, size, pipeline);
        });
}

void RenderService::WaitAll()
{
    m_impl->m_device->WaitForIdle();
}

void RenderService::OnResize(glm::ivec2 extent)
{
    if (extent.x == 0 || extent.y == 0)
    {
        core::log::warning("[RenderService] Resize with zeroed dimension was requested, clamping it to 1: {}x{}", extent.x, extent.y);
    }
    m_impl->m_resizeRequested = true;
    m_impl->m_newResolution = { glm::clamp<int>(extent.x, 1u, C_MAX_RESOLUTION), glm::clamp<int>(extent.y, 1u, C_MAX_RESOLUTION) };
}

void RenderService::OnWindowResize(glm::ivec2 extent)
{
    PROFILER_CPU_ZONE;

    WaitAll();
    m_impl->m_device->OnResize(extent.x, extent.y);
    CreateWindowResources(extent);

    if ((Instance().Cfg().m_domain & Domain::EDITOR) != Domain::EDITOR)
    {
        OnResize(extent);
    }
}

// TODO: Reconsider that method, maybe we can do that in a better way
glm::ivec2 RenderService::ViewportSize() const
{
    if ((Instance().Cfg().m_domain & engine::Domain::EDITOR) == engine::Domain::EDITOR)
    {
        return Instance().Service<EditorService>().ViewportSize();
    }

    if ((Instance().Cfg().m_domain & engine::Domain::CLIENT) == engine::Domain::CLIENT)
    {
        return Instance().Service<WindowService>().Extent();
    }

    ENGINE_ASSERT(false);
    return {};
}

const RPtr<rhi::RenderPass>& RenderService::ImGuiPass() const
{
    return m_impl->m_imguiRenderPass;
}

const RPtr<MaterialResource>& RenderService::DefaultMaterial() const
{
    return m_impl->m_renderMaterial;
}

const ResPtr<rhi::Pipeline>& RenderService::Pipeline(const ResPtr<MaterialResource>& res) const
{
    std::lock_guard l(m_impl->m_pipelineGetterMutex);

    auto& materialLoader = Instance().Service<ResourceService>().GetLoader<MaterialLoader>();

    return materialLoader.Pipeline(res);
}

void RenderService::CreateRenderResources(glm::ivec2 extent)
{
    PROFILER_CPU_ZONE;

    if (const auto imguiService = Instance().FindService<ImguiService>(); imguiService && m_impl->m_texture)
    {
        imguiService->RemoveImage(m_impl->m_texture);
    }

    GetMaterialLoader().ResizePipelines(extent);
}

void RenderService::CreateWindowResources(glm::ivec2 extent)
{
    PROFILER_CPU_ZONE;

    auto& matLoader = GetMaterialLoader();

    matLoader.ResizePipelines(extent, true);

    rhi::TextureDescriptor textureDescriptor;
    textureDescriptor.m_width = static_cast<uint16_t>(extent.x);
    textureDescriptor.m_height = static_cast<uint16_t>(extent.y);
    textureDescriptor.m_type = rhi::TextureType::TEXTURE_2D;
    textureDescriptor.m_layersAmount = 1;
    textureDescriptor.m_format = rhi::Format::BGRA8_UNORM;

    const auto imguiTex = CreateTexture(textureDescriptor);

    rhi::RenderPassDescriptor imguiPassDesc{};
    imguiPassDesc.m_extent = extent;
    imguiPassDesc.m_name = "ImGui";

    rhi::AttachmentDescriptor desc{};
    desc.m_texture = imguiTex;

    imguiPassDesc.m_colorAttachments.emplace_back(desc);

    m_impl->m_imguiRenderPass = CreateRenderPass(imguiPassDesc);

    // TODO: Add here rendering from PBR pass to swapchain in non-editor mode
    m_impl->m_presentMaterial->Material()->SetTexture(imguiTex, 0);
    //m_impl->m_presentMaterial->Material()->SetTexture(Pipeline(DefaultMaterial())->Descriptor().m_pass->Descriptor().m_colorAttachments[0].m_texture, 0);
    m_impl->m_presentMaterial->Material()->Sync();
}

void RenderService::LoadSystemResources()
{
    auto& matLoader = Instance().Service<ResourceService>().GetLoader<MaterialLoader>();

    m_impl->m_renderMaterial = matLoader.RenderMaterial();
    m_impl->m_presentMaterial = matLoader.PresentMaterial();

    ENGINE_ASSERT(m_impl->m_renderMaterial->Ready());
    ENGINE_ASSERT(m_impl->m_presentMaterial->Ready());

    auto albedoTex = Instance().Service<ResourceService>().Load<TextureResource>("/Textures/brick.png");
    albedoTex->Wait();

    m_impl->m_renderMaterial->Material()->SetBuffer<CameraUB>(0, rhi::ShaderStage::VERTEX, "CameraUB");
    m_impl->m_renderMaterial->Material()->SetTexture(albedoTex->Texture(), 1);
    m_impl->m_renderMaterial->Material()->Sync();

    rhi::BufferDescriptor presentVBDesc{};
    presentVBDesc.m_size = sizeof(presentVBRaw[0]) * static_cast<uint32_t>(presentVBRaw.size());
    presentVBDesc.m_memoryType = rhi::MemoryType::CPU_GPU;
    presentVBDesc.m_name = "PresentVB";
    presentVBDesc.m_type = rhi::BufferType::VERTEX;

    m_impl->m_presentVB = CreateBuffer(presentVBDesc, presentVBRaw.data());
}

} // engine
