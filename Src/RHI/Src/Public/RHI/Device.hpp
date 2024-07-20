#pragma once

#include <RHI/Config.hpp>
#include <RHI/TextureDescriptor.hpp>
#include <RHI/SamplerDescriptor.hpp>
#include <RHI/ShaderCompiler.hpp>
#include <RHI/IContext.hpp>
#include <RHI/RenderPassDescriptor.hpp>
#include <RHI/PipelineDescriptor.hpp>
#include <Core/Type.hpp>

namespace rhi
{

class Buffer;
class Shader;
class Sampler;
class Texture;
class RenderPass;
class Pipeline;
class GPUMaterial;
struct ComputeState;

class RHI_API Device : public core::NonCopyable
{
public:
    Device() : m_parameters()
    {}
    virtual ~Device() = default;

    virtual std::shared_ptr<ShaderCompiler>     CreateShaderCompiler(const ShaderCompiler::Options& options = {}) = 0;
    virtual std::shared_ptr<Buffer>             CreateBuffer(const BufferDescriptor& desc, const void* data) = 0;
    virtual std::shared_ptr<Shader>             CreateShader(const ShaderDescriptor& desc) = 0;
    virtual std::shared_ptr<Sampler>            CreateSampler(const SamplerDescriptor& desc) = 0;
    virtual std::shared_ptr<Texture>            CreateTexture(const TextureDescriptor& desc, const std::shared_ptr<Sampler>& sampler, const void* data = {}) = 0;
    virtual std::shared_ptr<RenderPass>         CreateRenderPass(const RenderPassDescriptor& desc) = 0;
    virtual std::shared_ptr<Pipeline>           CreatePipeline(const PipelineDescriptor& desc) = 0;
    virtual std::shared_ptr<GPUMaterial>        CreateGPUMaterial(const std::shared_ptr<Shader>& shader) = 0;

    virtual void                                BeginFrame() = 0;
    virtual void                                EndFrame() = 0;
    virtual void                                Present() = 0;
    virtual void                                BeginPipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                EndPipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                BeginComputePipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                EndComputePipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual std::shared_ptr<ComputeState>       BeginComputePipelineImmediate(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                EndComputePipeline(const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<ComputeState>& state) = 0;
    virtual void                                Draw(const std::shared_ptr<Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount) = 0;
    virtual void                                Draw(const std::shared_ptr<Buffer>& vb, const std::shared_ptr<Buffer>& ib, uint32_t indexCount, uint32_t instanceCount) = 0;
    virtual void                                Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    virtual void                                Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ, const std::shared_ptr<ComputeState>& state) = 0;
    virtual void                                BindGPUMaterial(const std::shared_ptr<GPUMaterial>& material, const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                BindGPUMaterial(const std::shared_ptr<GPUMaterial>& material, const std::shared_ptr<Pipeline>& pipeline, const std::shared_ptr<ComputeState>& state) = 0;
    virtual void                                PushConstant(const void* data, uint32_t size, const std::shared_ptr<Pipeline>& pipeline) = 0;

    virtual void                                OnResize(uint32_t x, uint32_t y) = 0;

    virtual void                                WaitForIdle() = 0;

    static std::shared_ptr<Device>              Create(const std::shared_ptr<IContext>& ctx);

    struct Parameters
    {
        uint32_t    m_minUniformBufferAlignment = 64;
        uint8_t     m_framesInFlight = 1;
        float       m_maxSamplerAnisotropy = 0;
    };

    Parameters m_parameters;
};

}
