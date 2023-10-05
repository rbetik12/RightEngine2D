cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME EngineNew)

find_package(glfw3 REQUIRED)
find_package(taskflow REQUIRED)
find_package(argparse)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
        )

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC
                            argparse::argparse
                            glfw
                            Taskflow::Taskflow
                            RHI
                    )

target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_EXPORT)
target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/PCH.hpp")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Source/")
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/.bin)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /WX)
endif()

# Create source groups for each directory
foreach(FILE ${SOURCE_FILES})
    # Get the path relative to the source directory
    file(RELATIVE_PATH RELATIVE_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})
    # Get the directory of the file
    get_filename_component(DIR ${RELATIVE_FILE} DIRECTORY)
    # Create the source group
    source_group(${DIR} FILES ${FILE})
endforeach()
