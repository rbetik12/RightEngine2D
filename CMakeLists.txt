cmake_minimum_required(VERSION 3.19)
project(RightEngine)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/.bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/.bin)

add_compile_definitions(R_ENABLE_ASSERTS)
if (WIN32)
    add_compile_definitions(R_WIN32)
elseif (APPLE)
    add_compile_definitions(R_APPLE)
endif()

if (MSVC)
    add_compile_options("/MP")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (WIN32)
    set(Vulkan_LIBRARY $ENV{Vulkan_LIBRARY})
    find_package(Vulkan REQUIRED)
elseif (APPLE)
    find_package(Vulkan REQUIRED)
endif()

message(STATUS "Vulkan include directory: " ${Vulkan_INCLUDE_DIR})
message(STATUS "Vulkan library directory: " ${Vulkan_LIBRARY})

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(Lib/easyargs/include)
include_directories(Lib/vk-tools/)
include_directories(Lib/VulkanMemoryAllocator/include)
include_directories(Lib/crossguid/include)
include_directories(Lib/yaml-cpp/include)

add_subdirectory(Lib/easyargs)
add_subdirectory(Lib/vk-tools/)
add_subdirectory(Lib/VulkanMemoryAllocator)

set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Enable yaml-cpp contrib in library")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Enable parse tools")
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build as shared library")
set(YAML_CPP_INSTALL OFF CACHE BOOL "Enable generation of yaml-cpp install targets")
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "Format source")
add_subdirectory(Lib/yaml-cpp)

set(CROSSGUID_TESTS OFF CACHE BOOL "Build test runner")
add_subdirectory(Lib/crossguid)


set(THIRD_PARTY_LIB
        crossguid
        easyargs
        yaml-cpp
        )

set(THIRD_PARTY_PROJECT_DIR_NAME "Lib")
set(CUSTOM_TARGET_DIR_NAME "Custom")

set_target_properties(yaml-cpp PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(crossguid PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(easyargs PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(uninstall PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(vk-tools PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})

add_subdirectory(Editor)
add_subdirectory(Engine)
add_subdirectory(Core)
add_subdirectory(RHI)
