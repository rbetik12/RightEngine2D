#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Engine.hpp>
#include <rttr/registration>

RTTR_REGISTRATION
{
rttr::registration::class_<engine::EditorService>("engine::EditorService")
	.constructor();
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
};

void EditorService::Update(float dt)
{
	
}

void EditorService::PostUpdate(float dt)
{
	
}

}