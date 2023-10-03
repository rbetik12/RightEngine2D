#include <Engine/Service/ServiceManager.hpp>

namespace engine
{

void ServiceManager::Update(float dt)
{
	for (auto& service : m_services)
	{
		service->Update(dt);
	}
}

void ServiceManager::PostUpdate(float dt)
{
	for (auto& service : m_services)
	{
		service->PostUpdate(dt);
	}
}

void ServiceManager::Destroy()
{
	std::reverse(m_services.begin(), m_services.end());

	for (auto& service: m_services)
	{
		const auto type = service->get_type();
		service.reset();
		core::log::debug("[ServiceManager] Destroyed service '{}' successfully", type.get_name());
	}
}

} // namespace engine