cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/RHI/Source/*.cpp"
        "${CMAKE_SOURCE_DIR}/RHI/Source/*.hpp"
        )

find_package(glm REQUIRED)
find_package(glslang REQUIRED)
find_package(spirv-cross REQUIRED)

add_library(RHI SHARED ${SOURCE_FILES})

target_compile_definitions(RHI PRIVATE RHI_EXPORT)
target_precompile_headers(RHI PRIVATE "${CMAKE_SOURCE_DIR}/RHI/Source/PCH.hpp")
target_include_directories(RHI PUBLIC "${CMAKE_SOURCE_DIR}/RHI/Source/Public")
target_include_directories(RHI PRIVATE "${CMAKE_SOURCE_DIR}/Lib/VulkanMemoryAllocator/include")
target_link_directories(RHI PRIVATE ${CMAKE_BINARY_DIR}/.bin)

target_link_libraries(RHI PUBLIC 
                            Core 
                            spirv-cross::spirv-cross
                            glslang::glslang
                            glslang::SPIRV
                            glm::glm
                            ${Vulkan_LIBRARY} 
                            VulkanMemoryAllocator
                            vk-tools
                    )

if (MSVC)
    target_compile_options(RHI PRIVATE /WX)
endif()

# Create source groups for each directory
foreach(FILE ${SOURCE_FILES})
    # Get the path relative to the source directory
    file(RELATIVE_PATH RELATIVE_FILE ${CMAKE_SOURCE_DIR} ${FILE})
    # Get the directory of the file
    get_filename_component(DIR ${RELATIVE_FILE} DIRECTORY)
    # Create the source group
    source_group(${DIR} FILES ${FILE})
endforeach()
