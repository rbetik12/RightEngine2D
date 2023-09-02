#pragma once

#include <chrono>

namespace RightEngine
{
    class Timer
	{
    public:
        Timer();

        void Start();
        void Stop();

        double TimeInMilliseconds();
        double TimeInSeconds();

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;
        bool m_running = false;
    };
}
