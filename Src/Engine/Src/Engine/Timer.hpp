#pragma once

#include <Engine/Config.hpp>
#include <chrono>

namespace engine
{

class ENGINE_API Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;

    inline Timer()
    {
        Start();
    }

    inline void Start()
    {
        m_startTime = Clock::now();
        m_running = true;
    }

    inline void Stop()
    {
        m_endTime = Clock::now();
        m_running = false;
    }

    inline void Restart()
    {
        m_startTime = m_endTime = Clock::now();
        m_running = true;
    }

    inline float TimeInMilliseconds()
    {
        std::chrono::time_point<Clock> endTime;

        if (m_running)
        {
            endTime = std::chrono::high_resolution_clock::now();
        }
        else
        {
            endTime = m_endTime;
        }

        return static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count());
    }

    inline float TimeInSeconds()
    {
        return TimeInMilliseconds() / 1000.0f;
    }

private:
    std::chrono::time_point<Clock>  m_startTime;
    std::chrono::time_point<Clock>  m_endTime;
    bool                            m_running = false;
};

} // engine