#include <Engine/Service/RenderService.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Engine.hpp>
#include <glslang/Public/ShaderLang.h>

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

    if (glslang::InitializeProcess())
    {
        core::log::info("[RenderService] Initialized glslang");
    }
    else
    {
        core::log::error("[RenderService] Can't initialize glslang");
    }
}

RenderService::~RenderService()
{
    glslang::FinalizeProcess();
}

void RenderService::Update(float dt)
{
}

void RenderService::PostUpdate(float dt)
{
}

std::shared_ptr<rhi::ShaderCompiler> RenderService::CreateShaderCompiler(const rhi::ShaderCompiler::Options& options)
{
    return m_device->CreateShaderCompiler(options);
}

std::shared_ptr<rhi::Buffer> RenderService::CreateBuffer(const rhi::BufferDescriptor& desc, const void* data)
{
    return m_device->CreateBuffer(desc, data);
}

std::shared_ptr<rhi::Shader> RenderService::CreateShader(const rhi::ShaderDescriptor& desc)
{
    return m_device->CreateShader(desc);
}

std::shared_ptr<rhi::Sampler> RenderService::CreateSampler(const rhi::SamplerDescriptor& desc)
{
    return m_device->CreateSampler(desc);
}

}
