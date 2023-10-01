#pragma once

#include <Engine/Config.hpp>

namespace engine
{
    class ENGINE_API Timer
    {
    public:
        Timer()
        {
            Start();
        }

        void Start()
        {
            m_startTime = std::chrono::high_resolution_clock::now();
            m_running = true;
        }

        void Stop()
        {
            m_endTime = std::chrono::high_resolution_clock::now();
            m_running = false;
        }

        float TimeInMilliseconds()
        {
            std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

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

        float TimeInSeconds()
        {
            return TimeInMilliseconds() / 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;
        bool                                                        m_running = false;
    };
}