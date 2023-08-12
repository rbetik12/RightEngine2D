cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/.bin)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/Engine/Source/*.cpp"
        "${CMAKE_SOURCE_DIR}/Engine/Source/*.hpp"
        "${CMAKE_SOURCE_DIR}/Engine/Assets/Shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/Engine/Assets/Shaders/*.frag"
        )

add_library(Engine STATIC ${SOURCE_FILES})
target_link_libraries(Engine PUBLIC RHI ${THIRD_PARTY_LIB})

set(ENGINE_INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/Engine/Source/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Event/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Launch/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Window/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Application/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/RHI/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Testing/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Shader/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Utils/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Scene/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/UI/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Loaders/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Core/Service/Public
)

target_include_directories(Engine PUBLIC ${ENGINE_INCLUDE_DIR})
target_include_directories(Engine INTERFACE ${ENGINE_INCLUDE_DIR})

# Create source groups for each directory
foreach(FILE ${SOURCE_FILES})
    # Get the path relative to the source directory
    file(RELATIVE_PATH RELATIVE_FILE ${CMAKE_SOURCE_DIR} ${FILE})
    # Get the directory of the file
    get_filename_component(DIR ${RELATIVE_FILE} DIRECTORY)
    # Create the source group
    source_group(${DIR} FILES ${FILE})
endforeach()

set(ENGINE_ASSETS_DIR ${CMAKE_SOURCE_DIR}/Engine/Assets)
add_compile_definitions("ENGINE_ASSETS_DIR=\"${ENGINE_ASSETS_DIR}\"")
