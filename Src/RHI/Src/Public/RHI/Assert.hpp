#pragma once

#include <iostream>

#ifdef R_ENABLE_ASSERTS
#ifdef WIN32
#define RHI_ASSERT(x) do { if(!(x)) { std::cout << "[RHI] Assertion failed: " << #x << std::endl; __debugbreak(); } } while(false)
#define RHI_ASSERT_WITH_MESSAGE(x, ...) do { if(!(x)) { std::cout << "[RHI] Assertion failed: " << #x << " " << __VA_ARGS__ << " " << std::endl; __debugbreak(); } } while(false)
#endif
#else
#define RHI_ASSERT(x, ...)
#endif