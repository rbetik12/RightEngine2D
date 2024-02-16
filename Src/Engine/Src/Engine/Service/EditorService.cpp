#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/Imgui/ImguiService.hpp>
#include <RHI/Shader.hpp>
#include <imgui.h>

RTTR_REGISTRATION
{
	engine::registration::Service<engine::EditorService>("engine::EditorService")
		.UpdateAfter<engine::ImguiService>()
		.PostUpdateBefore<engine::ImguiService>()
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
	ImGui::ShowDemoWindow();
	ImGui::Begin("Test");
	ImGui::Text("");
	ImGui::End();
}

void EditorService::PostUpdate(float dt)
{
	
}

}