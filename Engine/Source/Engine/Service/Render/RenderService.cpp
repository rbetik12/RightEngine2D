#include "RenderService.hpp"

#include <glslang/Public/ShaderLang.h>

#include "Application.hpp"

namespace engine::render
{

	void RenderService::OnRegister()
	{
		auto& ts = RightEngine::Instance().Service<ThreadService>();
		m_executor = ts.CreateExecutor("Render Thread", 1);

		
		// m_executor->async([]()
		// {
		// 	glslang::InitializeProcess();
		// });

		m_device = rhi::Device::Create();
		m_shaderCompiler = CreateShaderCompiler({});
	}

	RenderService::~RenderService()
	{
		// m_executor->async([]()
		// {
		// 	glslang::FinalizeProcess();
		// });
	}

	void RenderService::OnUpdate(float dt)
	{
	}

	std::future<rhi::ShaderData> RenderService::CompileShader(std::string_view path)
	{
		return m_executor->async([=]()
		{
			return m_shaderCompiler->Compile(path);
		});
	}

	std::shared_ptr<rhi::ShaderCompiler> RenderService::CreateShaderCompiler(const rhi::ShaderCompiler::Options& options)
	{
		return m_device->CreateShaderCompiler(options);
	}
}
