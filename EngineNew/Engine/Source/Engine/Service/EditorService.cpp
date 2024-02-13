#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/Render/Material.hpp>
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

namespace engine
{

EditorService::EditorService()
{
	auto& ts = Instance().Service<ThreadService>();
	ts.AddBackgroundTask([]()
		{
			core::log::debug("[EditorService] Message from another thread!");
		});
}

EditorService::~EditorService()
{
}

void EditorService::Update(float dt)
{
}

void EditorService::PostUpdate(float dt)
{
	
}

}