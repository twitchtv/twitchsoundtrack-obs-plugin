cmake_minimum_required(VERSION 3.12)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmakeUtils;")
include(setup)

project(soundtrack-plugin)

set(MSVC_DYNAMIC_RUNTIME OFF CACHE BOOL "" FORCE)
mark_as_advanced(MSVC_DYNAMIC_RUNTIME)

setup_project()

add_subdirectory(soundtrack)

if(BUILD_TESTING)
  # add_subdirectory()
endif()

# this generates the format all target
format_all(formatall)
