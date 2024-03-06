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

class RHI_API Device : public core::NonCopyable
{
public:
    virtual ~Device() = default;

    virtual std::shared_ptr<ShaderCompiler>     CreateShaderCompiler(const ShaderCompiler::Options& options = {}) = 0;
    virtual std::shared_ptr<Buffer>             CreateBuffer(const BufferDescriptor& desc, const void* data) = 0;
    virtual std::shared_ptr<Shader>             CreateShader(const ShaderDescriptor& desc) = 0;
    virtual std::shared_ptr<Sampler>            CreateSampler(const SamplerDescriptor& desc) = 0;
    virtual std::shared_ptr<Texture>            CreateTexture(const TextureDescriptor& desc, const std::shared_ptr<Sampler>& sampler, const void* data = {}) = 0;
    virtual std::shared_ptr<RenderPass>         CreateRenderPass(const RenderPassDescriptor& desc) = 0;
    virtual std::shared_ptr<Pipeline>           CreatePipeline(const PipelineDescriptor& desc) = 0;

    virtual void                                BeginFrame() = 0;
    virtual void                                EndFrame() = 0;
    virtual void                                Present() = 0;
    virtual void                                BeginPipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                EndPipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                BeginComputePipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                EndComputePipeline(const std::shared_ptr<Pipeline>& pipeline) = 0;
    virtual void                                Draw(const std::shared_ptr<Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount) = 0;
    virtual void                                Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    virtual void                                OnResize(uint32_t x, uint32_t y) = 0;

    virtual void                                WaitForIdle() = 0;

    static std::shared_ptr<Device>              Create(const std::shared_ptr<IContext>& ctx);
};

}
