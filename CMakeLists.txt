cmake_minimum_required (VERSION 3.8)

project (HelloDX12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set (CMAKE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/CMake")

# All external libraries from git modules
add_subdirectory("${PROJECT_SOURCE_DIR}/External")

# The actual source code
add_subdirectory("${PROJECT_SOURCE_DIR}/HelloDX12")

# Put CMake targets to a folder called "CMakeTargets"
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")
set_property(TARGET uninstall PROPERTY FOLDER "CMakeTargets")