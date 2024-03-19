cmake_minimum_required(VERSION 3.17)
project(vk-tools)

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    set(Vulkan_LIBRARY $ENV{Vulkan_LIBRARY})
    find_package(Vulkan REQUIRED)
elseif (APPLE)
    find_package(Vulkan REQUIRED)
endif()

set(VK_TOOLS_SRC
        vk-tools/VulkanTools.cpp
        )

add_library(vk-tools STATIC ${VK_TOOLS_SRC})

target_include_directories(vk-tools PRIVATE ${Vulkan_INCLUDE_DIR})