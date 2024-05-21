#include <Core/Thread.hpp>

namespace core
{

void ThreadIdStorage::SetMainThreadId(std::thread::id id)
{
    m_mainThreadId = id;
}

void ThreadIdStorage::SetRenderThreadId(std::thread::id id)
{
    m_renderThreadId = id;
}

std::thread::id ThreadIdStorage::RenderThreadId()
{
    return m_renderThreadId;
}

std::thread::id ThreadIdStorage::MainThreadId()
{
    return m_mainThreadId;
}

} // core