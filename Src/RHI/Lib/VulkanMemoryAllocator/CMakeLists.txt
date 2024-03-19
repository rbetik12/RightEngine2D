cmake_minimum_required(VERSION 3.17)
project(VulkanMemoryAllocator)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    set(Vulkan_LIBRARY $ENV{Vulkan_LIBRARY})
    find_package(Vulkan REQUIRED)
elseif (APPLE)
    find_package(Vulkan REQUIRED)
endif()

add_library(VulkanMemoryAllocator STATIC vk_mem_alloc.cpp)

target_include_directories(VulkanMemoryAllocator PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(VulkanMemoryAllocator PUBLIC include)

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

        /wd4189
        /wd4127
        /wd4324
    )
endif()
