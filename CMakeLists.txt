cmake_minimum_required (VERSION 3.19.0)
project (3DPrimitives)

set (3DP_ROOT_DIR ${PROJECT_SOURCE_DIR})

include(${3DP_ROOT_DIR}/CMake/HelperFunctions.cmake)

add_subdirectory(App)
add_subdirectory(Renderer)