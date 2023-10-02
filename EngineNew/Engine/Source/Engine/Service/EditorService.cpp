#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Engine.hpp>

namespace engine
{

EditorService::EditorService()
{
	auto& ts = Instance().Service<ThreadService>();
	ts.AddBackgroundTask([]()
		{
			core::log::debug("[EditorService] Message from another thread!");
		});
	core::log::info("Successfully initialized EditorService");
}

EditorService::~EditorService()
{
	core::log::info("Successfully destroyed EditorService");
};

void EditorService::Update(float dt)
{
	
}

void EditorService::PostUpdate(float dt)
{
	
}

}