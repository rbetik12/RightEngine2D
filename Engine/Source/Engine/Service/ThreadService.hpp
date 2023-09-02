#pragma once

#include "IService.hpp"
#include <taskflow/taskflow.hpp>

namespace engine
{
	class CustomExecutor : public tf::Executor
	{
	public:
		friend class ThreadService;
	private:
		CustomExecutor() = default;

		CustomExecutor(std::string_view name, size_t threadAmount) : m_name(name), tf::Executor(threadAmount)
		{}

		std::string m_name;
	};

	class ThreadService : public IService
	{
	public:
		template <typename F>
		auto AddBackgroundTask(F&& f)
		{
			return m_executor.async(std::move(f));
		}

		tf::Future<void> AddBackgroundTaskflow(tf::Taskflow&& taskflow);

		inline std::shared_ptr<CustomExecutor> CreateExecutor(std::string_view name, size_t threadAmount)
		{
			return std::shared_ptr<CustomExecutor>(new CustomExecutor(name, threadAmount));
		}

		virtual void OnRegister() override;
		virtual void OnUpdate(float dt) override;

		virtual ~ThreadService() override;

	private:
		tf::Executor			m_executor;
		std::list<tf::Taskflow> m_taskflows;
		std::mutex				m_mutex;
	};
}
