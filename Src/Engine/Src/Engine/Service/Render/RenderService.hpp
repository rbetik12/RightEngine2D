#pragma once

#include <Engine/Service/IService.hpp>
#include <RHI/Device.hpp>

namespace engine
{

template<typename T>
using RPtr = std::shared_ptr<T>;

class ENGINE_API RenderService final : public IService
{
    RTTR_ENABLE(IService);
public:
    RenderService();
    virtual ~RenderService() override;

    virtual void                Update(float dt) override;
    virtual void                PostUpdate(float dt) override;

    RPtr<rhi::ShaderCompiler>   CreateShaderCompiler(const rhi::ShaderCompiler::Options& options = {});
    RPtr<rhi::Buffer>           CreateBuffer(const rhi::BufferDescriptor& desc, const void* data = nullptr);
    RPtr<rhi::Texture>          CreateTexture(const rhi::TextureDescriptor& desc, const std::shared_ptr<rhi::Sampler>& sampler = {}, const void* data = nullptr);
    RPtr<rhi::Shader>           CreateShader(const rhi::ShaderDescriptor& desc);
    RPtr<rhi::Sampler>          CreateSampler(const rhi::SamplerDescriptor& desc);
    RPtr<rhi::RenderPass>       CreateRenderPass(const rhi::RenderPassDescriptor& desc);
    RPtr<rhi::Pipeline>         CreatePipeline(const rhi::PipelineDescriptor& desc);

    void                        BeginPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        EndPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        Draw(const std::shared_ptr<rhi::Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount = 1);

    void                        WaitAll();
    void                        OnResize(uint32_t width, uint32_t height);

    // Called automatically on every window resize event, will resize fullscreen passes, like present or ImGui
    // Will resize other renderpasses in non-editor mode
    // Don't call it manually unless you really know what are you doing!!!
    void                        OnWindowResize(uint32_t width, uint32_t height);

    // TODO: Should be removed after all material and passes systems will be done
    const RPtr<rhi::RenderPass>& BasicPass() const;
    const RPtr<rhi::RenderPass>& ImGuiPass() const;

    const RPtr<rhi::ShaderCompiler>& ShaderCompiler() const;
    const RPtr<rhi::Shader>&    DefaultShader() const;
    const RPtr<rhi::Pipeline>&  DefaultPipeline() const;

private:
    struct Impl;

    void                        CreateRenderResources(uint32_t width, uint32_t height);
    void                        CreateWindowResources(uint32_t width, uint32_t height);
    void                        LoadSystemResources();

    std::shared_ptr<rhi::Device>    m_device;
    std::shared_ptr<rhi::Sampler>   m_defaultSampler;
    std::shared_ptr<rhi::IContext>  m_context;
    std::unique_ptr<Impl>           m_impl;
};

} // engine