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
    add_compile_options(
        /W4              # Warning level 4
        #/WX              # Enable warnings as errors
        /wd4624            # Disable warning: 'derived class' : destructor was implicitly defined as deleted because a base class destructor is inaccessible or deleted
        /wd4251            # Disable warning: class 'A' needs to have dll-interface to be used by clients of class 'B'
        /wd4307            # Disable warning: '*': integral constant overflow
        /wd4100            # Disable warning: '*' : unreferenced formal parameter
        /wd4275            # Disable warning: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
        /wd4661            # Disable warning: '*' : no suitable definition provided for explicit template instantiation request
        /w44189            # Enable warning: '*' : local variable is initialized but not referenced
        /w44263            # Enable warning: 'function': member function does not override any base class virtual member function
        /w44264            # Enable warning: 'virtual_function': no override available for virtual member function from base 'class'; function is hidden
        /w44265            # Enable warning: 'class': class has virtual functions, but destructor is not virtual
        /w44296            # Enable warning: 'operator': expression is always false
        /w44464            # Enable warning: relative include path contains '..'
        /w44555            # Enable warning: expression has no effect; expected expression with side-effect
        /w45038            # Enable warning: data member 'member1' will be initialized after data member 'member2'
        /w45204            # Enable warning: A class with virtual functions has non-virtual trivial destructor.
        /w45233            # Enable warning: explicit lambda capture 'identifier' is not used
        /w45245            # Enable warning: 'function': unreferenced function with internal linkage has been removed
        /w45249            # Enable warning: 'bitfield' of type 'enumeration_name' has named enumerators with values that cannot be represented in the given bit field width of 'bitfield_width'    
        /MP
        /Gy              # Enable function level linking
        /Gw              # Optimize Global Data
        #/GR-            # Disable Run-Time Type Information
        /bigobj            # Increase number of sections in object files
        /Oi              # Enable intrinsics, irrelevant of optimization level
        /fp:fast          # Enable fast floating point arithmetics by default
        /permissive-        # Enable standards conformance mode to the compiler
        /Zc:__cplusplus        # Enable reporting real C++ version by __cplusplus macro - necessary for some libraries
        /Zc:lambda          # Enable updated lambda processor
        /Zc:preprocessor      # Enable preprocessor conformance mode
    )
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
