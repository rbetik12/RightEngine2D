#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/RenderService.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <RHI/Texture.hpp>

RTTR_REGISTRATION
{
engine::registration::Service<engine::EditorService>("engine::EditorService")
	.Domain(engine::Domain::EDITOR);
}


namespace engine
{

EditorService::EditorService()
{
	auto& ts = Instance().Service<ThreadService>();
	ts.AddBackgroundTask([]()
		{
			core::log::debug("[EditorService] Message from another thread!");
		});

	auto& rs = Instance().Service<RenderService>();
	m_shaderCompiler = rs.CreateShaderCompiler();

	const auto shaderPath = "C:\\Users\\Vitaliy\\code\\RightEngine\\Resources\\Shaders\\basic.glsl";
	const auto compiledShader = m_shaderCompiler->Compile(shaderPath);

	rhi::ShaderDescriptor shaderDescriptor;
	shaderDescriptor.m_path = shaderPath;
	shaderDescriptor.m_name = "Basic";
	shaderDescriptor.m_blobByStage = compiledShader.m_stageBlob;
	shaderDescriptor.m_reflection = compiledShader.m_reflection;
	shaderDescriptor.m_type = rhi::ShaderType::FX;
	const auto shader = rs.CreateShader(shaderDescriptor);


	rhi::TextureDescriptor textureDescriptor;
	textureDescriptor.m_width = 1024;
	textureDescriptor.m_height = 1024;
	textureDescriptor.m_componentAmount = 4;
	textureDescriptor.m_type = rhi::TextureType::TEXTURE_2D;
	textureDescriptor.m_layersAmount = 1;
	textureDescriptor.m_format = rhi::Format::RGBA8_UINT;

	const auto texture = rs.CreateTexture(textureDescriptor);
}

EditorService::~EditorService()
{
};

void EditorService::Update(float dt)
{
	
}

void EditorService::PostUpdate(float dt)
{
	
}

}