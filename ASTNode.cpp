//
// Created by Michal Young on 9/12/18.
//

#include "ASTNode.h"

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Each node type needs a str method -- most are in .h file for inlining,
    // a few require more code.


    // Binary operators have an 'eval' method
    int Plus::eval(EvalContext& ctx) { return left.eval(ctx) + right.eval(ctx); }
    int Times::eval(EvalContext& ctx) { return left.eval(ctx) * right.eval(ctx); }
    int Minus::eval(EvalContext& ctx) { return left.eval(ctx) - right.eval(ctx); }
    int Div::eval(EvalContext& ctx) { return left.eval(ctx) / right.eval(ctx); }

    // A block is evaluated just by evaluating each statement in the block.
    // We'll return the value of the last statement, although it is useless.
    // The value of an empty block is zero.
    int Block::eval(EvalContext& ctx) {
        int result = 0;
        for (auto &s: stmts) {
            result = s->eval(ctx);
        }
        return result;
    }


    // Identifiers live in symtab and default to 0.
    int Ident::eval(EvalContext &ctx) {
        if (ctx.symtab.count(this->text) == 1) {
            return ctx.symtab[this->text];
        } else {
            return 0;
        }
    }

    // Assignment evaluates its right hand side and stores the
    // result into its left hand side.  We'll have it return the
    // value it produced just for simplicity and debugging, but the
    // value is not otherwise used.
    int Assign::eval(EvalContext &ctx) {
        std::string loc = lexpr.l_eval(ctx);
        int rvalue = rexpr.eval(ctx);
        ctx.symtab[loc] = rvalue;
        return rvalue;
    }
}