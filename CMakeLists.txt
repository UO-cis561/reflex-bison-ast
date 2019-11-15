# CMake script for building very simple parser
# with bison and 'reflex' scanner generator.
#
#
cmake_minimum_required(VERSION 3.13)
project(AST
        VERSION 0.1
        DESCRIPTION "Demo of parsing and AST building for CIS 461/561"
        LANGUAGES CXX
        )
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_COMPILER g++)

# No effect:
# set(PROJECT_BINARY_DIR "build" CACHE PATH "I want binaries together in top-level directory")
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})


# Out-of-source build in 'build' directory, targeting 'bin' directory
add_subdirectory(src "${CMAKE_SOURCE_DIR}/build")
