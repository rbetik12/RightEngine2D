#pragma once

#include <Engine/Service/IService.hpp>
#include <RHI/Device.hpp>

namespace engine
{

class ENGINE_API RenderService final : public IService
{
	RTTR_ENABLE(IService);
public:
	RenderService();
	virtual ~RenderService() override;

	virtual void							Update(float dt) override;
	virtual void							PostUpdate(float dt) override;

	std::shared_ptr<rhi::ShaderCompiler>	CreateShaderCompiler(const rhi::ShaderCompiler::Options& options = {});
	std::shared_ptr<rhi::Buffer>			CreateBuffer(const rhi::BufferDescriptor& desc, const void* data = nullptr);
	std::shared_ptr<rhi::Texture>			CreateTexture(const rhi::TextureDescriptor& desc, const std::shared_ptr<rhi::Sampler>& sampler = {}, const void* data = nullptr);
	std::shared_ptr<rhi::Shader>			CreateShader(const rhi::ShaderDescriptor& desc);
	std::shared_ptr<rhi::Sampler>			CreateSampler(const rhi::SamplerDescriptor& desc);
	std::shared_ptr<rhi::RenderPass>		CreateRenderPass(const rhi::RenderPassDescriptor& desc);
	std::shared_ptr<rhi::Pipeline>			CreatePipeline(const rhi::PipelineDescriptor& desc);

	void									BeginPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
	void									EndPass(const std::shared_ptr<rhi::Pipeline>& pipeline);
	void									Draw(const std::shared_ptr<rhi::Buffer>& buffer, uint32_t vertexCount, uint32_t instanceCount = 1);

	void									WaitAll();
	void									OnResize(uint32_t weight, uint32_t height);

	// TODO: Should be removed after all material and passes systems will be done
	const std::shared_ptr<rhi::RenderPass>& ImGuiPass() const;

private:

	void									CreateRenderResources(uint32_t width, uint32_t height);
	void									LoadSystemResources();
	struct Impl;

	std::shared_ptr<rhi::Device>	m_device;
	std::shared_ptr<rhi::Sampler>	m_defaultSampler;
	std::shared_ptr<rhi::IContext>	m_context;
	std::unique_ptr<Impl> m_impl;
};

} // engine