#pragma once

#include <tracy/Tracy.hpp>
#include <common/TracySystem.hpp>

#ifdef R_ENABLE_TRACY
#define PROFILER_CPU_ZONE ZoneScoped
#define PROFILER_FRAME_END FrameMark
#define PROFILER_SET_THREAD_NAME(NAME) tracy::SetThreadName(NAME)
#else
#define PROFILER_CPU_ZONE
#define PROFILER_FRAME_END
#define PROFILER_SET_THREAD_NAME(NAME)
#endif