project (nano_imgui)
cmake_minimum_required(VERSION 2.8)

# deps
include(cmake/deps.cmake)

# macro and configuration
include(cmake/config.cmake)

# projects
add_subdirectory(src)