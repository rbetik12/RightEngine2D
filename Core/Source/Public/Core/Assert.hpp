#pragma once

#pragma once

#include <iostream>
#include <cassert>

#ifdef R_ENABLE_ASSERTS
#ifdef WIN32
#define CORE_ASSERT(x) do { if(!(x)) { std::cout << "[CORE] Assertion failed: " << #x << std::endl; assert(x); } } while(false)
#define CORE_ASSERT_WITH_MESSAGE(x, ...) do { if(!(x)) { std::cout << "[CORE] Assertion failed: " << #x << __VA_ARGS__ << " " << std::endl; assert(x); } } while(false)
#endif
#else
#define RHI_ASSERT(x, ...)
#endif
