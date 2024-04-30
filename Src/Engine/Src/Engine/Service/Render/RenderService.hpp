#pragma once

#include <Engine/Service/IService.hpp>
#include <RHI/Device.hpp>

namespace engine
{

namespace render
{
class Material;
} // render

template<typename T>
using RPtr = std::shared_ptr<T>;

class ENGINE_API RenderService final : public IService
{
    RTTR_ENABLE(IService)
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
    RPtr<rhi::GPUMaterial>      CreateGPUMaterial(const std::shared_ptr<rhi::Shader>& shader);

    void                        BeginPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        EndPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        BeginComputePass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        EndComputePass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        Draw(const std::shared_ptr<rhi::Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount = 1);
    void                        Draw(const std::shared_ptr<rhi::Buffer>& vb, const std::shared_ptr<rhi::Buffer>& ib, uint32_t instanceCount = 1);
    void                        Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void                        BindMaterial(const std::shared_ptr<render::Material>& material, const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        PushConstant(const void* data, uint32_t size, const std::shared_ptr<rhi::Pipeline>& pipeline);

    void                        WaitAll();
    void                        OnResize(uint32_t width, uint32_t height);

    // Called automatically on every window resize event, will resize fullscreen passes, like present or ImGui
    // Will resize other renderpasses in non-editor mode
    // Don't call it manually unless you really know what are you doing!!!
    void                        OnWindowResize(uint32_t width, uint32_t height);

    glm::ivec2                  ViewportSize() const;

    // TODO: Should be removed after all material and passes systems will be done
    const RPtr<rhi::RenderPass>&        BasicPass() const;
    const RPtr<rhi::RenderPass>&        ImGuiPass() const;

    const RPtr<rhi::Shader>&            DefaultShader() const;
    const RPtr<render::Material>&       DefaultMaterial() const;
    const RPtr<rhi::Pipeline>&          DefaultPipeline() const;

    friend class Engine;

private:
    struct Impl;

    void                        CreateRenderResources(uint32_t width, uint32_t height);
    void                        CreateWindowResources(uint32_t width, uint32_t height);
    void                        LoadSystemResources();

    std::unique_ptr<Impl> m_impl;
};

} // engine