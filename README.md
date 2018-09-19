# reflex-bison-ast
Simple example showing how re-flex and bison can build an AST

Another try to RE/flex and bison working together smoothly in C++, this time based on a RE/flex example.

## Building

This project is designed to be built using cmake.  CMake should build a subdirectory called cmake-build-debug, where intermediate files and compiled code is placed.  I have so far done this only within the Clion IDE from JetBrains, and I'm not
completely sure which part of the "build" process is handled by CMake and whether some of the build process is
actually specific to CLion.

## Files
* ASTNode.h, ASTnode.cpp:  These are the abstract syntax tree.  We want to keep it as simple as possible, but no simpler.
* calc.lxx, calc.yxx:  The RE/flex and Bison source files, respectively.  calc.yxx will be translated by bison into several files: calc.tab.hxx, calc.tab.cxx, location.hh, position.hh, stack.hh.  calc.lxx depends on some of those header files, which describe how tokens, semantic values (e.g., the name of an identifier), and position information are communication between parser and scanner.  calc.lxx is translated by RE/flex (command 'reflex') into lex.yy.h and lex.yy.cpp.  (There is obviously no consistency in the filename extensions used for header and C++ code.)
* Messages.h and Messages.cpp are an attempt to factor error reporting out of the parser and lexer code.  It is not entirely successful because the ways we access information about positions varies from place to place.
* CMakeLists.txt is like a Makefile but all meta and stuff so that CMake can build either a standard Makefile for Unix or some kind of scripty something for Windows.  Don't hate me, I'm just trying to use the build system required for CLion, and learning as I go.
* EvalContext.h  environment structure we need to pass around to evaluate an AST.  For this simple example it's just a hashmap.
* CodegenContext.{h,cpp} incomplete --- this will become the context object to be passed around during code generation, but it's not written yet.
* sample.txt  A small input file that I use for smoke tests (not thorough testing, just checking that it's not completely busted)
* test_ast.cpp  Snippets of code I use to check the AST when it is too hard to debug within the parser.  Often I use this to resolve type errors that I don't understand.  Very often.  Because I'm basically trying to learn C++ by writing this parser. What did I think that was a good idea?
* parser.cpp  The driver (main program) for the parser build from the bison (.yxx) and reflex (.lxx) sources.
* run.sh  Since CLion can't redirect input (what?!),  I use this tiny shell script to pipe a named file into stdin. 

## Notes

While flex has an option that redefines yylex to return instances of a class, RE/flex does not.  Its documentation declares that doing this redefinition by macros is "ugly", but does not say more about why.  Too bad.

Instead, yylex always returns an int, as in the original lex and flex, but we can pass objects to yylex to be filled in.  Which seems uglier, because we have to make an assignment into the passed yylval and yyloc objects (with absolutely no check that we have remembered to do so).  Speaking of ugly ...

