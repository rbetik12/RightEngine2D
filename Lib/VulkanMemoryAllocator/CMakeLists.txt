cmake_minimum_required(VERSION 3.17)
project(VulkanMemoryAllocator)

include_directories(include)

add_library(VulkanMemoryAllocator vk_mem_alloc.cpp)

if (MSVC)
    target_compile_options(VulkanMemoryAllocator PRIVATE
        /WX-
        /MP                # Enable parallel compilation
        /Gy                # Enable function level linking
        /Gw                # Optimize Global Data
        /GR-               # Disable Run-Time Type Information
        /Oi                # Enable intrinsics, irrelevant of optimization level
        /fp:fast           # Enable fast floating point arithmetics by default
        /Zc:__cplusplus    # Enable reporting real C++ version by __cplusplus macro - necessary for some libraries
        /Zc:lambda         # Enable updated lambda processor
    )
endif()
