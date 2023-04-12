cmake_minimum_required(VERSION 3.16)

PROJECT(CppGL
    VERSION 1.7
    DESCRIPTION "OpenGL framework for use in teaching and research with focus on simplicity and reusability."
    LANGUAGES CXX
)

if (NOT UNIX AND NOT WIN32)
	message(FATAL_ERROR "Unsupported platform!")
endif()

# ---------------------------------------------------------------------
# cmake options

option(CPPGL_BUILD_GLM "" ON)
option(CPPGL_BUILD_EXAMPLES "" OFF)

# ---------------------------------------------------------------------
# path management

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# ---------------------------------------------------------------------
# subtrees

set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_SHARED ON CACHE BOOL "" FORCE)
add_subdirectory(subtrees/glew)
include_directories(subtrees/glew/include)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_USE_CHDIR OFF CACHE BOOL " " FORCE)
add_subdirectory(subtrees/glfw)
include_directories(subtrees/glfw/include)

if (CPPGL_BUILD_GLM)
    add_subdirectory(subtrees/glm EXCLUDE_FROM_ALL)
    include_directories(subtrees/glm)
endif()

find_package(assimp QUIET)
if (NOT assimp_FOUND)
    message(STATUS "ASSIMP NOT FOUND: compiling from source")
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(subtrees/assimp)
    include_directories(subtrees/assimp/include)
else()
    message(STATUS "ASSIMP FOUND: ${assimp_DIR}")
    include_directories(${assimp_INCLUDE_DIR})
endif()

# ---------------------------------------------------------------------
# compiler options

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# setup for included dependencies
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_LINK_DIRECTORIES_BEFORE ON)

if(UNIX)
    add_compile_options(-pthread -march=native)
else()
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING) # wtf
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(GenerateExportHeader)

# ---------------------------------------------------------------------
# traverse source tree

include_directories(src)
add_subdirectory(src)

if (CPPGL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
