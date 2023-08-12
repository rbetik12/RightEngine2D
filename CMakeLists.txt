cmake_minimum_required(VERSION 3.19)
project(RightEngine)

set(CMAKE_CXX_STANDARD 17)

enable_testing()
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

set(GLSL_ROOT_DIR ${CMAKE_SOURCE_DIR}/Engine/Source/Shaders)
set(GLSL_SRC ${GLSL_ROOT_DIR}/Basic/basic.frag
             ${GLSL_ROOT_DIR}/Basic/basic.vert
             ${GLSL_ROOT_DIR}/Basic/pbr.frag
             ${GLSL_ROOT_DIR}/Basic/pbr.vert
             ${GLSL_ROOT_DIR}/Basic/hdr_to_cubemap.frag
             ${GLSL_ROOT_DIR}/Basic/hdr_to_cubemap.vert
             ${GLSL_ROOT_DIR}/Basic/skybox.frag
             ${GLSL_ROOT_DIR}/Basic/skybox.vert
             ${GLSL_ROOT_DIR}/Utils/envmap_to_cubemap.frag
             ${GLSL_ROOT_DIR}/Utils/envmap_to_cubemap.vert
        )

if (WIN32)
    set(Vulkan_LIBRARY $ENV{Vulkan_LIBRARY})
    find_package(Vulkan REQUIRED FATAL_ERROR)
elseif (APPLE)
    find_package(Vulkan REQUIRED FATAL_ERROR)
endif()

message(STATUS "Vulkan include directory: " ${Vulkan_INCLUDE_DIR})
message(STATUS "Vulkan library directory: " ${Vulkan_LIBRARY})

include_directories(before system Lib/glslang/)
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(Lib/spdlog/include)
include_directories(Lib/easyargs/include)
include_directories(Lib/googletest/googletest/include)
include_directories(Lib/glfw/include)
include_directories(Lib/glm)
include_directories(Lib/stb_image/Public)
include_directories(Lib/imgui)
include_directories(Lib/entt/include)
include_directories(Lib/assimp/include)
include_directories(${CMAKE_BINARY_DIR}/Lib/assimp/include)
include_directories(Lib/vk-tools/)
include_directories(Lib/VulkanMemoryAllocator/include)
include_directories(Lib/crossguid/include)
include_directories(Lib/hash-library)
include_directories(Lib/ImGuizmo)
include_directories(Lib/yaml-cpp/include)
include_directories(Lib/taskflow)
include_directories(Lib/EASTL/include)

add_subdirectory(Lib/spdlog)
add_subdirectory(Lib/easyargs)
add_subdirectory(Lib/glm)
add_subdirectory(Lib/stb_image)
add_subdirectory(Lib/imgui)
add_subdirectory(Lib/glslang)
add_subdirectory(Lib/vk-tools/)
add_subdirectory(Lib/VulkanMemoryAllocator)
add_subdirectory(Lib/hash-library)
add_subdirectory(Lib/ImGuizmo)

#set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Enable yaml-cpp contrib in library")
#set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Enable parse tools")
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build as shared library")
#set(YAML_CPP_INSTALL OFF CACHE BOOL "Enable generation of yaml-cpp install targets")
#set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "Format source")
add_subdirectory(Lib/yaml-cpp)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
add_subdirectory(Lib/glfw)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Build the assimp viewing tools")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build the assimp tests")
set(ASSIMP_INSTALL OFF CACHE BOOL "Install assimp as separate lib")
add_subdirectory(Lib/assimp)

set(CROSSGUID_TESTS OFF CACHE BOOL "Build test runner")
add_subdirectory(Lib/crossguid)

set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest.")
add_subdirectory(Lib/googletest)

set(TF_BUILD_BENCHMARKS OFF CACHE BOOL "Enables builds of benchmarks")
set(TF_BUILD_CUDA OFF CACHE BOOL "Enables builds of cuda code")
set(TF_BUILD_SYCL OFF CACHE BOOL "Enables builds of sycl code")
set(TF_BUILD_TESTS OFF CACHE BOOL "Enables builds of tests")
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "Enables builds of examples")
add_subdirectory(Lib/taskflow)

add_subdirectory(Lib/simdjson)

add_subdirectory(Lib/EASTL)

set(THIRD_PARTY_LIB
        glfw
        stb_image
        imgui
        assimp
        crossguid
        easyargs
        ImGuizmo
        yaml-cpp
        Taskflow
        )

set(THIRD_PARTY_PROJECT_DIR_NAME "Lib")
set(CUSTOM_TARGET_DIR_NAME "Custom")

set_target_properties(glfw PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(stb_image PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(imgui PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(assimp PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(zlibstatic PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(ImGuizmo PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(yaml-cpp PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(yaml-cpp-parse PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(yaml-cpp-read PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(yaml-cpp-sandbox PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(crossguid PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(easyargs PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(gtest PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(gtest_main PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(hash-library PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(spdlog PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(uninstall PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(Taskflow PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(simdjson PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(EASTL PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(vk-tools PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})

add_subdirectory(Editor)
add_subdirectory(Engine)
add_subdirectory(Core)
add_subdirectory(RHI)
