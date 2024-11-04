#pragma once

#include <Engine/Service/IService.hpp>
#include <taskflow/taskflow.hpp>

namespace engine
{

class CustomThread;

class ENGINE_API ThreadService final : public Service<ThreadService>
{
public:

    ThreadService();
    virtual ~ThreadService() override;

    virtual void                    Update(float dt) override;
    virtual void                    PostUpdate(float dt) override;

    template <typename F>
    auto                            AddBackgroundTask(F&& f)
    {
        return m_bgExecutor->async(std::move(f));
    }

    tf::Future<void>                AddBackgroundTaskflow(tf::Taskflow&& taskflow);

    tf::Future<void>                AddForegroundTaskflow(tf::Taskflow& taskflow);

    std::unique_ptr<CustomThread>   SpawnThread(std::string_view name);

    // Consider using SpawnThread at first time
    std::shared_ptr<tf::Executor>   NamedExecutor(std::string_view name, int threadAmount) const;

private:
    std::unique_ptr<tf::Executor>    m_bgExecutor;
    std::unique_ptr<tf::Executor>    m_fgExecutor;
    std::list<tf::Taskflow>          m_taskflows;
    std::mutex                       m_mutex;
};

class ENGINE_API CustomThread : core::NonCopyable
{
public:
    CustomThread(std::string_view name);
    ~CustomThread();

    template <typename F>
    auto AddTask(F&& f)
    {
        return m_executor->async(std::move(f));
    }

    void WaitForAll();

    friend class ThreadService;

private:
    std::shared_ptr<tf::Executor> m_executor;
};

} // namespace engine