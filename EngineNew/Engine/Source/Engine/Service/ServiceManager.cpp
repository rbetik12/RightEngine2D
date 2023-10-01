#include <Engine/Service/ServiceManager.hpp>

namespace engine
{

void ServiceManager::Update(float dt)
{
	for (auto& [_, service] : m_services)
	{
		service->Update(dt);
	}
}

void ServiceManager::PostUpdate(float dt)
{
	for (auto& [_, service] : m_services)
	{
		service->PostUpdate(dt);
	}
}

void ServiceManager::Destroy()
{
	for (auto& [_, service] : m_services)
	{
		service.reset();
	}
}

} // namespace engine