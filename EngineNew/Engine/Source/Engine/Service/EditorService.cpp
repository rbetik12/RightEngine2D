#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/RenderService.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <RHI/Texture.hpp>
#include <RHI/PipelineDescriptor.hpp>
#include <RHI/Buffer.hpp>
#include <RHI/Pipeline.hpp>
#include <RHI/Shader.hpp>

RTTR_REGISTRATION
{
engine::registration::Service<engine::EditorService>("engine::EditorService")
	.Domain(engine::Domain::EDITOR);
}

namespace
{

const eastl::vector<float> vertexBufferRaw = 
{
	-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
};

} // namespace unnamed

namespace engine
{

struct EditorService::Impl
{
	std::shared_ptr<rhi::ShaderCompiler>	m_shaderCompiler;
	std::shared_ptr<rhi::Shader>			m_shader;
	std::shared_ptr<rhi::Buffer>			m_buffer;
	std::shared_ptr<rhi::Texture>			m_texture;
	std::shared_ptr<rhi::RenderPass>		m_renderPass;
	std::shared_ptr<rhi::Pipeline>			m_pipeline;
};

EditorService::EditorService()
{
	m_impl = std::make_unique<Impl>();

	auto& ts = Instance().Service<ThreadService>();
	ts.AddBackgroundTask([]()
		{
			core::log::debug("[EditorService] Message from another thread!");
		});

	auto& rs = Instance().Service<RenderService>();
	m_impl->m_shaderCompiler = rs.CreateShaderCompiler();

	const auto shaderPath = "C:\\Users\\Vitaliy\\code\\RightEngine\\Resources\\Shaders\\basic.glsl";
	const auto compiledShader = m_impl->m_shaderCompiler->Compile(shaderPath);

	rhi::ShaderDescriptor shaderDescriptor;
	shaderDescriptor.m_path = shaderPath;
	shaderDescriptor.m_name = "Basic";
	shaderDescriptor.m_blobByStage = compiledShader.m_stageBlob;
	shaderDescriptor.m_reflection = compiledShader.m_reflection;
	shaderDescriptor.m_type = rhi::ShaderType::FX;
	m_impl->m_shader = rs.CreateShader(shaderDescriptor);

	rhi::TextureDescriptor textureDescriptor;
	textureDescriptor.m_width = 1024;
	textureDescriptor.m_height = 1024;
	textureDescriptor.m_componentAmount = 4;
	textureDescriptor.m_type = rhi::TextureType::TEXTURE_2D;
	textureDescriptor.m_layersAmount = 1;
	textureDescriptor.m_format = rhi::Format::RGBA8_UINT;

	m_impl->m_texture = rs.CreateTexture(textureDescriptor);

	rhi::RenderPassDescriptor renderPassDescriptor;
	renderPassDescriptor.m_extent = { 1024, 1024 };
	renderPassDescriptor.m_name = "Test";
	renderPassDescriptor.m_colorAttachments = { {m_impl->m_texture} };

	m_impl->m_renderPass = rs.CreateRenderPass(renderPassDescriptor);

	rhi::PipelineDescriptor pipelineDescriptor{};
	pipelineDescriptor.m_pass = m_impl->m_renderPass;
	pipelineDescriptor.m_shader = m_impl->m_shader;
	pipelineDescriptor.m_cullMode = rhi::CullMode::NONE;

	m_impl->m_pipeline = rs.CreatePipeline(pipelineDescriptor);

	rhi::BufferDescriptor bufferDesc{};
	bufferDesc.m_memoryType = rhi::MemoryType::CPU_GPU;
	bufferDesc.m_type = rhi::BufferType::VERTEX;
	bufferDesc.m_size = sizeof(vertexBufferRaw[0]) * static_cast<uint32_t>(vertexBufferRaw.size());

	m_impl->m_buffer = rs.CreateBuffer(bufferDesc, vertexBufferRaw.data());
}

EditorService::~EditorService()
{
};

void EditorService::Update(float dt)
{
	auto& rs = Instance().Service<RenderService>();
	rs.BeginPass(m_impl->m_pipeline);
	rs.Draw(m_impl->m_buffer, m_impl->m_buffer->Descriptor().m_size /
		m_impl->m_pipeline->Descriptor().m_shader->Descriptor().m_reflection.m_inputLayout.Stride());
	rs.EndPass(m_impl->m_pipeline);
}

void EditorService::PostUpdate(float dt)
{
	
}

}