cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/Engine/Source/*.cpp"
        "${CMAKE_SOURCE_DIR}/Engine/Source/*.hpp"
        "${CMAKE_SOURCE_DIR}/Engine/Assets/Shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/Engine/Assets/Shaders/*.frag"
        )

find_package(taskflow REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(stb REQUIRED)
find_package(imgui REQUIRED)
find_package(imguizmo REQUIRED)
find_package(EnTT REQUIRED)

add_library(Engine STATIC ${SOURCE_FILES})
target_link_libraries(Engine PUBLIC 
                            RHI 
                            ${THIRD_PARTY_LIB} 
                            assimp::assimp 
                            glfw 
                            Taskflow::Taskflow
                            stb::stb
                            imgui::imgui
                            imguizmo::imguizmo
                            EnTT::EnTT)

set(ENGINE_INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/Engine/Source/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Event/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Launch/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Window/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Application/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/RHI/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Testing/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Shader/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Utils/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Scene/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/UI/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/Loaders/Public
    ${CMAKE_SOURCE_DIR}/Engine/Source/
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
