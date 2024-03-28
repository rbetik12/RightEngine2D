cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h"
        )

find_package(glslang REQUIRED)
find_package(spirv-cross REQUIRED)
find_package(imgui REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(Lib/vk-tools/)
add_subdirectory(Lib/VulkanMemoryAllocator)
set_target_properties(vk-tools PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})
set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER ${THIRD_PARTY_PROJECT_DIR_NAME})

add_library(RHI STATIC ${SOURCE_FILES})

target_link_libraries(RHI PUBLIC 
                            Core 
                            spirv-cross::spirv-cross
                            glslang::glslang
                            glslang::SPIRV
                            imgui::imgui
                            vulkan-headers::vulkan-headers
                            ${Vulkan_LIBRARY} 
                            VulkanMemoryAllocator
                            vk-tools
                    )

target_compile_definitions(RHI PRIVATE RHI_EXPORT)
target_precompile_headers(RHI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/PCH.hpp")

target_include_directories(RHI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Src/Public")
target_include_directories(RHI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/")
target_include_directories(RHI PRIVATE "${CMAKE_SOURCE_DIR}/Lib/VulkanMemoryAllocator/include")
target_include_directories(RHI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Lib/vk-tools")

target_link_directories(RHI PRIVATE ${CMAKE_BINARY_DIR}/.bin)

# Create source groups for each directory
foreach(FILE ${SOURCE_FILES})
    # Get the path relative to the source directory
    file(RELATIVE_PATH RELATIVE_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})
    # Get the directory of the file
    get_filename_component(DIR ${RELATIVE_FILE} DIRECTORY)
    # Create the source group
    source_group(${DIR} FILES ${FILE})
endforeach()
