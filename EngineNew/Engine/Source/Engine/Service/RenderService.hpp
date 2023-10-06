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

	virtual void									Update(float dt) override;
	virtual void									PostUpdate(float dt) override;

	std::shared_ptr<rhi::ShaderCompiler>	CreateShaderCompiler(const rhi::ShaderCompiler::Options& options = {});
	std::shared_ptr<rhi::Buffer>			CreateBuffer(const rhi::BufferDescriptor& desc, const void* data);
	std::shared_ptr<rhi::Shader>			CreateShader(const rhi::ShaderDescriptor& desc);
	std::shared_ptr<rhi::Sampler>			CreateSampler(const rhi::SamplerDescriptor& desc);

private:
	std::shared_ptr<rhi::Device>	m_device;
	std::shared_ptr<rhi::IContext>	m_context;
};

} // namespace engine