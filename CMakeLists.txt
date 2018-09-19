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

# Recipes for generating C++ source files from .lxx and .yxx files
#

add_custom_command(
        OUTPUT lex.yy.cpp lex.yy.h
        COMMAND   reflex --bison-cc --bison-locations --header-file ${CMAKE_CURRENT_SOURCE_DIR}/calc.lxx
        DEPENDS calc.lxx calc.tab.hxx
)
add_custom_command(
        OUTPUT calc.tab.cxx calc.tab.hxx location.hh position.hh stack.hh
        COMMAND bison -d ${CMAKE_CURRENT_SOURCE_DIR}/calc.yxx
        DEPENDS calc.yxx
)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${REFLEX_INCLUDE}
        ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(parser
        calc.tab.cxx lex.yy.cpp lex.yy.h
        parser.cpp ASTNode.cpp ASTNode.h Messages.h Messages.cpp CodegenContext.cpp CodegenContext.h EvalContext.h)

add_executable(test_ast  test_ast.cpp
        ASTNode.cpp ASTNode.h CodegenContext.cpp CodegenContext.h)

target_link_libraries(parser ${REFLEX_LIB})