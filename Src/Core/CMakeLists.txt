cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp"
        )

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(EASTL REQUIRED)
find_package(rttr REQUIRED)
find_package(Tracy REQUIRED)
find_package(glm REQUIRED)

add_library(Core SHARED ${SOURCE_FILES})

target_sources(Core PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/EASTL.natvis")
target_sources(Core PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/rttr.natvis")
target_sources(Core PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/NatvisHolder.natvis")

target_link_libraries(Core PUBLIC 
                            EASTL::EASTL 
                            spdlog::spdlog
                            fmt::fmt 
                            RTTR::Core 
                            Tracy::TracyClient
                            glm::glm)
                            
target_compile_definitions(Core PRIVATE CORE_EXPORT)
target_precompile_headers(Core PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Src/PCH.hpp")

set(CORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Src/Public")
target_include_directories(Core INTERFACE ${CORE_INCLUDE_DIR})
target_include_directories(Core PUBLIC ${CORE_INCLUDE_DIR})

# Create source groups for each directory
foreach(FILE ${SOURCE_FILES})
    # Get the path relative to the source directory
    file(RELATIVE_PATH RELATIVE_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})
    # Get the directory of the file
    get_filename_component(DIR ${RELATIVE_FILE} DIRECTORY)
    # Create the source group
    source_group(${DIR} FILES ${FILE})
endforeach()
