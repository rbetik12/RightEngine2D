#include "Timer.hpp"

using namespace RightEngine;

Timer::Timer()
{
    Start();
}

void Timer::Start()
{
	m_startTime = std::chrono::high_resolution_clock::now();
	m_running = true;
}

void Timer::Stop()
{
	m_endTime = std::chrono::high_resolution_clock::now();
	m_running = false;
}

double Timer::TimeInMilliseconds()
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

    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count();
}

double Timer::TimeInSeconds()
{
    return TimeInMilliseconds() / 1000.0;
}
