//
// A context object for 'eval' methods.
// We need to carry a symbol table for variables that have
// been assigned values.  We could optionally carry around
// additional information, but for now just the symbols.
//

#ifndef AST_EVALCONTEXT_H
#define AST_EVALCONTEXT_H

#include <unordered_map>

// EvalContext is really just a struct for passing around the
// context.  There is no attempt at information hiding here.
//
class EvalContext {
public:
    std::unordered_map<std::string,int> symtab;
    explicit EvalContext() { }
};


#endif //AST_EVALCONTEXT_H
