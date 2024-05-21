#pragma once

#include <thread>

namespace core
{

class CORE_API ThreadIdStorage
{
public:
    static void SetMainThreadId(std::thread::id id);

    static void SetRenderThreadId(std::thread::id id);

    static std::thread::id RenderThreadId();

    static std::thread::id MainThreadId();

private:
    inline static std::thread::id m_mainThreadId;
    inline static std::thread::id m_renderThreadId;
};

inline std::thread::id CurrentThreadID()
{
    return std::this_thread::get_id();
}

inline bool IsMainThread()
{
    return CurrentThreadID() == ThreadIdStorage::MainThreadId();
}

inline bool IsRenderThread()
{
    return CurrentThreadID() == ThreadIdStorage::RenderThreadId();
}

} // core