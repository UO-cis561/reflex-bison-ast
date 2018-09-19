//
// A code generation context carries information that we need to maintain
// while walking the AST to generate code.  A context is passed along from
// from node to node as the tree is walked.
//
// For this small calculator example (with the rather impractical
// approach of evaluating some simple expressions by translating
// them to C code and then compiling and running the C code) we
// need an output stream and a table of variable values.
//

#ifndef AST_CODEGENCONTEXT_H
#define AST_CODEGENCONTEXT_H

#include <ostream>

class CodegenContext {
public:
    // We're not hiding implementation here, just
    // passing it around.
    std::ostream &stream;
    // Todo: Other context, like virtual "registers"
    explicit CodegenContext(std::ostream &out) : stream{out} {};
};


#endif //AST_CODEGENCONTEXT_H
