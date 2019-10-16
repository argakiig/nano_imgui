project (nano_imgui)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# deps
include(cmake/deps.cmake)

# macro and configuration
include(cmake/config.cmake)

# projects
add_subdirectory(src)