cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME EngineLauncher)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
        )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE EngineNew)

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