#include <Engine/Service/RenderService.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Engine.hpp>
#pragma warning(push)
#pragma warning(disable : 4464)
#include <glslang/Public/ShaderLang.h>
#pragma warning(pop)

RTTR_REGISTRATION
{
engine::registration::Service<engine::RenderService>("engine::RenderService")
    .Domain(engine::Domain::UI);
}

namespace engine
{

RenderService::RenderService()
{
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

    const auto extent = Instance().Service<WindowService>().Extent();
    m_device->OnResize(extent.x, extent.y);
}

RenderService::~RenderService()
{
    m_device.reset();
    m_context.reset();
}

void RenderService::Update(float dt)
{
    m_device->BeginFrame();
}

void RenderService::PostUpdate(float dt)
{
    m_device->EndFrame();
}

std::shared_ptr<rhi::ShaderCompiler> RenderService::CreateShaderCompiler(const rhi::ShaderCompiler::Options& options)
{
    return m_device->CreateShaderCompiler(options);
}

std::shared_ptr<rhi::Buffer> RenderService::CreateBuffer(const rhi::BufferDescriptor& desc, const void* data)
{
    return m_device->CreateBuffer(desc, data);
}

std::shared_ptr<rhi::Texture> RenderService::CreateTexture(const rhi::TextureDescriptor& desc, const void* data)
{
    return m_device->CreateTexture(desc, data);
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

}
