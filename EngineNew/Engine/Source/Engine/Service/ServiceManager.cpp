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
	eastl::vector<std::shared_ptr<IService>> servicesList;

	for (auto& [_, service] : m_services)
	{
		servicesList.push_back(service);
	}
	m_services.clear();

	std::reverse(servicesList.begin(), servicesList.end());

	for (auto& service: servicesList)
	{
		const auto type = service->get_type();
		service.reset();
		core::log::debug("[ServiceManager] Destroyed service '{}' successfully", type.get_name());
	}
}

} // namespace engine