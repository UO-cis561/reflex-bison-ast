//
// Does the ASTNode module work by itself?
//


#include <iostream>
#include "ASTNode.h"
#include "EvalContext.h"

using namespace AST;

void types_test() {
    LExpr *lexpr = new Ident("a");
    // LExpr &lref = Ident("b");  Can't because Ident("b") is a temporary
    LExpr& aref = *lexpr; // OK!
    Ident b = Ident("b");
    LExpr&  lref = b;  // OK!
    return;
}

// Instantiate all the concrete classes to make sure we
// have implemented all the virtual methods
void all_the_concrete_classes() {
    // class ASTNode is abstract

    // class LExpr is abstract
    // concrete extensions of LExpr
    Ident ident = Ident("a");

    // class BinOp is abstract
    // concrete extensions of BinOp
    Minus minus(ident, ident);
    Plus plus(ident, ident);
    Times times(ident, ident);
    Div div(ident, ident);

    // concrete extensions of ASTNode
    IntConst intconst = IntConst(5);
    Block block;
    Assign assign = Assign(ident, intconst);


    return;
}



int main(int argc, char **argv) {
    IntConst *x = new IntConst(5);
    IntConst *y = new IntConst(7);
    Plus  *e1 = new Plus(*x, *y);
    Ident *z = new Ident(std::string("z"));
    Plus *e = new Plus(*e1, *z);
    std::cout << e->str() << std::endl;
    auto twelve = new IntConst(12);
    // LExpr& lexpr_ = *twelve;  //! Integers aren't l-expressions
    Assign *assignment = new Assign(*z,*twelve);
    std::cout << "Evaluating " << assignment->str() << std::endl;
    EvalContext ctx;
    // std::cout << "Evaluates to " << assignment->eval(ctx) << std::endl;
}
