//
// Created by Michal Young on 9/12/18.
//

#include "ASTNode.h"

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Each node type needs a str method -- most are in .h file for inlining,
    // a few require more code.


    // Binary operators have an 'eval' method
    int Plus::eval(EvalContext &ctx) { return left_.eval(ctx) + right_.eval(ctx); }

    int Times::eval(EvalContext &ctx) { return left_.eval(ctx) * right_.eval(ctx); }

    int Minus::eval(EvalContext &ctx) { return left_.eval(ctx) - right_.eval(ctx); }

    int Div::eval(EvalContext &ctx) { return left_.eval(ctx) / right_.eval(ctx); }

    // A block is evaluated just by evaluating each statement in the block.
    // We'll return the value_ of the last statement, although it is useless.
    // The value_ of an empty block is zero.
    int Block::eval(EvalContext &ctx) {
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

    // JSON representation of all the concrete node types.
    // This might be particularly useful if I want to do some
    // tree manipulation in Python or another language.  We'll
    // do this by emitting into a stream.

    // --- Utility functions used by node-specific json output methods


    /* Indent to a given level */
    void ASTNode::json_indent(std::ostream& out, AST_print_context& ctx) {
        if (ctx.indent_ > 0) {
            out << std::endl;
        }
        for (int i=0; i < ctx.indent_; ++i) {
            out << "    ";
        }
    }

    /* The head element looks like { "kind" : "block", */
    void ASTNode::json_head(std::string node_kind, std::ostream& out, AST_print_context& ctx) {
        json_indent(out, ctx);
        out << "{ \"kind\" : \"" << node_kind << "\"," ;
        ctx.indent();  // one level more for children
        return;
    }

    void ASTNode::json_close(std::ostream& out, AST_print_context& ctx) {
        // json_indent(out, ctx);
        out << "}";
        ctx.dedent();
    }

    void ASTNode::json_child(std::string field, ASTNode& child, std::ostream& out, AST_print_context& ctx, char sep) {
        json_indent(out, ctx);
        out << "\"" << field << "\" : ";
        child.json(out, ctx);
        out << sep;
    }


    void Block::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Block", out, ctx);
        // Special case for list of children, but we probably we need to generalize this
        // for other "list of X" nodes, such as parameter lists in Quack.
        out << "\"stmts_\" : [";
        auto sep = "";
        for (ASTNode *stmt: stmts_) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_close(out, ctx);
    }

    void Assign::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Assign", out, ctx);
        json_child("lexpr_", lexpr_, out, ctx);
        json_child("rexpr_", rexpr_, out, ctx, ' ');
        json_close(out, ctx);
     }

    void If::json(std::ostream& out, AST_print_context& ctx) {
        json_head("If", out, ctx);
        json_child("cond_", cond_, out, ctx);
        json_child("truepart_", truepart_, out, ctx);
        json_child("falsepart_", falsepart_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void Ident::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Ident", out, ctx);
        out << "\"text_\" : \"" << text_ << "\"";
        json_close(out, ctx);
    }

    void IntConst::json(std::ostream& out, AST_print_context& ctx) {
        json_head("IntConst", out, ctx);
        out << "\"value_\" : " << value_ ;
        json_close(out, ctx);
    }

    void BinOp::json(std::ostream& out, AST_print_context& ctx) {
        json_head(opsym, out, ctx);
        json_child("left_", left_, out, ctx);
        json_child("right_", right_, out, ctx, ' ');
        json_close(out, ctx);
    }

}