#include <Engine/Service/ThreadService.hpp>
#include <Core/String.hpp>
#include <Engine/Registration.hpp>
#include <algorithm>

#include "Engine/Engine.hpp"

#if defined(R_WIN32)
#define NOMINMAX
#include <Windows.h>
#endif

RTTR_REGISTRATION
{
engine::registration::Service<engine::ThreadService>("engine::ThreadService")
    .Domain(engine::Domain::ALL);
}


namespace engine
{

auto CurrentThreadNativeHandle()
{
#if defined(R_WIN32)
    return GetCurrentThread();
#elif defined(R_APPLE)
    return pthread_self();
#else
    static_assert(false, "Not implemented!");
    return uint64_t{ 0 };
#endif
}

void SetThreadName(std::string_view name)
{
    thread_local bool nameSet = false;
    if (nameSet)
    {
        return;
    }
    nameSet = true;

    PROFILER_SET_THREAD_NAME(name.data());

#if defined(R_WIN32)
    using SetThreadDescriptionPtr = HRESULT(__stdcall*)(HANDLE, PCWSTR);
    static const auto SetThreadDescription = reinterpret_cast<SetThreadDescriptionPtr>(GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "SetThreadDescription"));

    if (!SetThreadDescription)
    {
        return;
    }

    const auto wName = core::string::Convert(name);

    SetThreadDescription(CurrentThreadNativeHandle(), wName.c_str());
#elif defined(COREX_PLATFORM_LINUX)
    pthread_setname_np(currentThreadNativeHandle(), name.data());
#elif defined(COREX_PLATFORM_APPLE)
    pthread_setname_np(name.data());
#endif
}

class WorkerInterface final : public tf::WorkerInterface
{
public:

    WorkerInterface(std::string_view threadNamePrefix, int workersCount) : m_threadNamePrefix(std::move(threadNamePrefix))
    {
        m_threads.resize(workersCount, std::thread::id());
    }

    bool containThread(const std::thread::id& id) const
    {
        return eastl::find(m_threads.begin(), m_threads.end(), id) != m_threads.end();
    }

private:
    void scheduler_prologue(tf::Worker& worker) override
    {
        if (m_threads.size() > 1)
        {
            SetThreadName(fmt::format("{} #{}", m_threadNamePrefix, worker.id()));
        }
        else
        {
            SetThreadName(fmt::format("{}", m_threadNamePrefix));
        }

        m_threads[worker.id()] = std::this_thread::get_id();
    }

    void scheduler_epilogue(tf::Worker& worker, std::exception_ptr ptr) override {}

private:
    std::string                     m_threadNamePrefix;
    eastl::vector<std::thread::id>  m_threads;
};

ThreadService::ThreadService()
{
    const auto workersAmount = 2;
    m_bgExecutor = std::make_unique<tf::Executor>(workersAmount, std::make_shared<WorkerInterface>("Background Thread", workersAmount));
    m_fgExecutor = std::make_unique<tf::Executor>(workersAmount, std::make_shared<WorkerInterface>("Foreground Thread", workersAmount));

    core::ThreadIdStorage::SetMainThreadId(core::CurrentThreadID());
}

ThreadService::~ThreadService()
{
}

void ThreadService::Update(float dt)
{
    PROFILER_CPU_ZONE;
}

void ThreadService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;
}

tf::Future<void> ThreadService::AddBackgroundTaskflow(tf::Taskflow&& taskflow)
{
    std::lock_guard lock(m_mutex);

    // TODO: Add some logic to update to clear taskflows list
    m_taskflows.emplace_back(std::move(taskflow));
    return m_bgExecutor->run(m_taskflows.back());
}

tf::Future<void> ThreadService::AddForegroundTaskflow(tf::Taskflow& taskflow)
{
    return m_fgExecutor->run(taskflow);
}

std::unique_ptr<CustomThread> ThreadService::SpawnThread(std::string_view name)
{
    return std::make_unique<CustomThread>(name);
}

std::shared_ptr<tf::Executor> ThreadService::NamedExecutor(std::string_view name, int threadAmount) const
{
    return std::make_shared<tf::Executor>(threadAmount, std::make_shared<WorkerInterface>(name, threadAmount));
}

CustomThread::~CustomThread()
{
    AddTask([]{});
    WaitForAll();
}

void CustomThread::WaitForAll()
{
    m_executor->wait_for_all();
}

CustomThread::CustomThread(std::string_view name)
{
    m_executor = Instance().Service<ThreadService>().NamedExecutor(name, 1);
}

} // namespace engine