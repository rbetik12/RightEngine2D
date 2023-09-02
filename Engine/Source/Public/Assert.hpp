#pragma once

#include "Logger.hpp"
#include <csignal>

#ifdef R_ENABLE_ASSERTS
#ifdef WIN32
#define R_ASSERT(x, ...) { if(!(x)) { R_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define R_CORE_ASSERT(x, ...) { if(!(x)) { R_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define R_ASSERT(x, ...) { if(!(x)) { R_ERROR("Assertion Failed: {0}", __VA_ARGS__); raise(SIGTRAP); } }
    #define R_CORE_ASSERT(x, ...) { if(!(x)) { R_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); raise(SIGTRAP); } }
#endif
#else
#define R_ASSERT(x, ...)
#define R_CORE_ASSERT(x, ...)
#endif