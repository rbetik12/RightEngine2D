#pragma once

#include <iostream>

#ifdef R_ENABLE_ASSERTS
#ifdef WIN32
#define ENGINE_ASSERT(x) do { if(!(x)) { std::cout << "[Engine] Assertion failed: " << #x << std::endl; __debugbreak(); } } while(false)
#define ENGINE_ASSERT_WITH_MESSAGE(x, ...) do { if(!(x)) { std::cout << "[Engine] Assertion failed: " << #x << __VA_ARGS__ << " " << std::endl; __debugbreak(); } } while(false)
#endif
#else
#define ENGINE_ASSERT(x, ...)
#endif