#pragma once

#include <Engine/Service/IService.hpp>
#include <RHI/Device.hpp>

namespace engine
{

namespace render
{
class Material;
} // render

class MaterialResource;

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

    void                        BeginPass(const ResPtr<MaterialResource>& material);
    void                        BeginPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        EndPass(const ResPtr<MaterialResource>& material);
    void                        EndPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
    void                        BeginComputePass(const ResPtr<MaterialResource>& material);
    RPtr<rhi::ComputeState>     BeginComputePassImmediate(const ResPtr<MaterialResource>& material);
    void                        EndComputePass(const ResPtr<MaterialResource>& material);
    void                        EndComputePass(const ResPtr<MaterialResource>& material, const RPtr<rhi::ComputeState>& state);
    void                        Draw(const std::shared_ptr<rhi::Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount = 1);
    void                        Draw(const std::shared_ptr<rhi::Buffer>& vb, const std::shared_ptr<rhi::Buffer>& ib, uint32_t instanceCount = 1);
    void                        Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void                        Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ, const RPtr<rhi::ComputeState>& state);
    void                        BindMaterial(const ResPtr<MaterialResource>& material);
    void                        BindMaterial(const ResPtr<MaterialResource>& material, const RPtr<rhi::ComputeState>& state);
    void                        PushConstant(const void* data, uint32_t size, const std::shared_ptr<rhi::Pipeline>& pipeline);

    void                        WaitAll();
    void                        OnResize(glm::ivec2 extent);

    // Called automatically on every window resize event, will resize fullscreen passes, like present or ImGui
    // Will resize other renderpasses in non-editor mode
    // Don't call it manually unless you really know what are you doing!!!
    void                        OnWindowResize(glm::ivec2 extent);

    glm::ivec2                  ViewportSize() const;

    const RPtr<rhi::RenderPass>&        ImGuiPass() const;

    const RPtr<MaterialResource>&       DefaultMaterial() const;

    const ResPtr<rhi::Pipeline>&        Pipeline(const ResPtr<MaterialResource>& res) const;

    template <typename F>
    auto RunOnRenderThread(F&& f)
    {
        return m_renderThread->AddTask(f);
    }

    template <typename F>
    auto RunOnRenderThreadWait(F&& f)
    {
        return m_renderThread->AddTask(f).get();
    }

    friend class Engine;

private:
    struct Impl;

    void                        CreateRenderResources(glm::ivec2 extent);
    void                        CreateWindowResources(glm::ivec2 extent);
    void                        LoadSystemResources();

    std::unique_ptr<Impl>           m_impl;
    std::unique_ptr<CustomThread>   m_renderThread;
};

} // engine