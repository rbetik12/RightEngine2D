cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/RHITest/Source/*.cpp"
        "${CMAKE_SOURCE_DIR}/RHITest/Source/*.hpp"
        )

add_executable(RHITest ${SOURCE_FILES})

target_link_libraries(RHITest PRIVATE 
                            glfw
                            RHI
                    )

if (MSVC)
    target_compile_options(RHITest PRIVATE /WX)
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
