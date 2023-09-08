#pragma once

#include <Engine/Service/IService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <RHI/Device.hpp>
#include <memory>

namespace engine::render
{

	class RenderService : public IService
	{
	public:
		virtual ~RenderService() override;

		virtual void OnRegister() override;

		virtual void OnUpdate(float dt) override;

		// Schedules shader compilation on render thread
		std::future<rhi::CompiledShaderData> CompileShader(std::string_view path);

	private:

		std::shared_ptr<rhi::ShaderCompiler> CreateShaderCompiler(const rhi::ShaderCompiler::Options& options = {});

		std::shared_ptr<rhi::Device>			m_device;
		std::shared_ptr<CustomExecutor>			m_executor;
		std::shared_ptr<rhi::ShaderCompiler>	m_shaderCompiler;
	};

}