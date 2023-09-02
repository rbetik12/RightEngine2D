#include "ThreadService.hpp"

namespace engine
{

	tf::Future<void> ThreadService::AddBackgroundTaskflow(tf::Taskflow&& taskflow)
	{
		std::lock_guard lock(m_mutex);
		m_taskflows.emplace_back(std::move(taskflow));
		return m_executor.run(m_taskflows.back());
	}

	void ThreadService::OnRegister()
	{
	}

	void ThreadService::OnUpdate(float dt)
	{}

	ThreadService::~ThreadService()
	{}

}