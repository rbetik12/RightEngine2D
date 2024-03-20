cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME Engine)

find_package(glfw3 REQUIRED)
find_package(taskflow REQUIRED)
find_package(argparse REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(EnTT REQUIRED)
find_package(stduuid REQUIRED)
find_package(stb REQUIRED)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h"
        )

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC
                            argparse::argparse
                            glfw
                            Taskflow::Taskflow
                            nlohmann_json::nlohmann_json
                            RHI
                            EnTT::EnTT
                            stduuid::stduuid
                            stb::stb
                    )

target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_EXPORT)
target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/PCH.hpp")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Src/")
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/.bin)

# Create source groups for each directory
foreach(FILE ${SOURCE_FILES})
    # Get the path relative to the source directory
    file(RELATIVE_PATH RELATIVE_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})
    # Get the directory of the file
    get_filename_component(DIR ${RELATIVE_FILE} DIRECTORY)
    # Create the source group
    source_group(${DIR} FILES ${FILE})
endforeach()
