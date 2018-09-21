//
// Created by Michal Young on 9/12/18.
//

#include "ASTNode.h"

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Each node type needs a str method -- most are in .h file for inlining,
    // a few require more code.


    // Binary operators have an 'eval' method
    int Plus::eval(EvalContext& ctx) { return left_.eval(ctx) + right_.eval(ctx); }
    int Times::eval(EvalContext& ctx) { return left_.eval(ctx) * right_.eval(ctx); }
    int Minus::eval(EvalContext& ctx) { return left_.eval(ctx) - right_.eval(ctx); }
    int Div::eval(EvalContext& ctx) { return left_.eval(ctx) / right_.eval(ctx); }

    // A block is evaluated just by evaluating each statement in the block.
    // We'll return the value_ of the last statement, although it is useless.
    // The value_ of an empty block is zero.
    int Block::eval(EvalContext& ctx) {
        int result = 0;
        for (auto &s: stmts_) {
            result = s->eval(ctx);
        }
        return result;
    }


    // Identifiers live in symtab and default to 0.
    int Ident::eval(EvalContext &ctx) {
        if (ctx.symtab.count(text_) == 1) {
            return ctx.symtab[text_];
        } else {
            return 0;
        }
    }

    // Assignment evaluates its right_ hand side and stores the
    // result into its left_ hand side.  We'll have it return the
    // value_ it produced just for simplicity and debugging, but the
    // value_ is not otherwise used.
    int Assign::eval(EvalContext &ctx) {
        std::string loc = lexpr_.l_eval(ctx);
        int rvalue = rexpr_.eval(ctx);
        ctx.symtab[loc] = rvalue;
        return rvalue;
    }

    // An 'if' statement, in this initial cut, evaluates its condition to an integer
    // and chooses the true (then) part or the false (else) part depending on whether
    // the integer is zero.
    int If::eval(EvalContext &ctx) {
        int cond = cond_.eval(ctx);
        // Might as well use C's ill-considered interpretation of ints as booleans
        if (cond) {
            return truepart_.eval(ctx);
        }
        return falsepart_.eval(ctx);
    }
}

