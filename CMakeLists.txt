# CMake script for building very simple parser
# with bison and 'reflex' scanner generator.
#
#
cmake_minimum_required(VERSION 3.12)
project(AST)
set(CMAKE_CXX_STANDARD 14)

# Where are the reflex include and library files?
#
set(REFLEX_LIB "/usr/local/lib/libreflex.a")
set(REFLEX_INCLUDE "/usr/local/include/reflex")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Recipes for generating C++ source files from .lxx and .yxx files
#

add_custom_command(
        OUTPUT lex.yy.cpp lex.yy.h
        COMMAND   reflex --bison-cc --bison-locations --header-file ${CMAKE_CURRENT_SOURCE_DIR}/quack.lxx
        DEPENDS quack.lxx quack.tab.hxx
)
add_custom_command(
        OUTPUT quack.tab.cxx quack.tab.hxx location.hh position.hh stack.hh
        COMMAND bison -d ${CMAKE_CURRENT_SOURCE_DIR}/quack.yxx
        DEPENDS quack.yxx
)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${REFLEX_INCLUDE}
        ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(parser
        quack.tab.cxx lex.yy.cpp lex.yy.h

        main.cpp parser.h TypeChecker.h TypeChecker.cpp AST.h AST.cpp Messages.h Messages.cpp CodegenContext.cpp CodegenContext.h)

target_link_libraries(parser ${REFLEX_LIB})
