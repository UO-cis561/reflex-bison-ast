//
// Created by Michal Young on 9/12/18.
//

#include "ASTNode.h"
#include <stdlib.h>

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Each node type needs a str method -- most are in .h file for inlining,
    // a few require more code.


    /* ============   Immediate Evaluation (Calculator Model) ================== */

    /* Binary operators */
    int Plus::eval(EvalContext &ctx) { return left_.eval(ctx) + right_.eval(ctx); }

    int Times::eval(EvalContext &ctx) { return left_.eval(ctx) * right_.eval(ctx); }

    int Minus::eval(EvalContext &ctx) { return left_.eval(ctx) - right_.eval(ctx); }

    int Div::eval(EvalContext &ctx) { return left_.eval(ctx) / right_.eval(ctx); }

    // C is already short-circuit for && and || so we can use them directly for calculator mode
    int And::eval(EvalContext &ctx) { return left_.eval(ctx) && right_.eval(ctx); }
    int Or::eval(EvalContext &ctx) { return left_.eval(ctx) || right_.eval(ctx); }
    int Not::eval(EvalContext &ctx) { return ! (left_.eval(ctx)); }

    /* Comparisons work like binary operators in calculator mode */
    int Less::eval(EvalContext &ctx) { return left_.eval(ctx) < right_.eval(ctx); }
    int AtMost::eval(EvalContext &ctx) { return left_.eval(ctx) <= right_.eval(ctx); }
    int AtLeast::eval(EvalContext &ctx) { return left_.eval(ctx) >= right_.eval(ctx); }
    int Greater::eval(EvalContext &ctx) { return left_.eval(ctx) > right_.eval(ctx); }
    int Equals::eval(EvalContext &ctx) { return left_.eval(ctx) == right_.eval(ctx); }

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

    int AsBool::eval(EvalContext &ctx) {
        // For calculator mode, we will just use the
        // arithmetic value as a boolean, as C does.
        return  left_.eval(ctx);
    }



    /* =================== Translation to C code (Compiler mode) ================ */
    void Block::gen_rvalue(CodegenContext& ctx, std::string target_reg) {
        for (auto &s: stmts_) {
            s->gen_rvalue(ctx, target_reg);
        }
    }

    void Assign::gen_rvalue(CodegenContext& ctx, std::string target_reg) {
        std::string loc = lexpr_.gen_lvalue(ctx);
        rexpr_.gen_rvalue(ctx, target_reg);
        /* Store the value in the location */
        ctx.emit(loc + "= " + target_reg + ";");
    }

    /* IF is a statement that executes either its true branch
     * or its false branch.  The value it places into the target
     * should be the value of whichever branch is taken.
     */
    void If::gen_rvalue(CodegenContext &ctx, std::string target_reg)  {
        std::string thenpart = ctx.new_branch_label("then");
        std::string elsepart = ctx.new_branch_label("else");
        std::string endpart = ctx.new_branch_label("endif");
        cond_.gen_branch(ctx, thenpart, elsepart);
        /* Generate the 'then' part here */
        ctx.emit(thenpart + ": ;");
        truepart_.gen_rvalue(ctx, target_reg);
        ctx.emit(std::string("goto ") + endpart + ";");
        /* Generate the 'else' part here */
        ctx.emit(elsepart + ": ;");
        falsepart_.gen_rvalue(ctx, target_reg);
        /* That's all, folks */
        ctx.emit(endpart + ": ;");
    }

    void Compare::gen_branch(CodegenContext &ctx, std::string true_branch, std::string false_branch) {
        std::string left_reg = ctx.alloc_reg();
        left_.gen_rvalue(ctx, left_reg);
        std::string right_reg = ctx.alloc_reg();
        right_.gen_rvalue(ctx, right_reg);
        ctx.emit(std::string("if (") + left_reg + c_compare_op_ + right_reg + ") goto " + true_branch + ";");
        ctx.emit(std::string("goto ") + false_branch + ";");
        ctx.free_reg(left_reg);
        ctx.free_reg(right_reg);
    }

    void And::gen_branch(CodegenContext &ctx, std::string true_branch, std::string false_branch) {
        std::string right_part = ctx.new_branch_label("and");
        left_.gen_branch(ctx, right_part, false_branch);
        ctx.emit(right_part + ": ;");
        right_.gen_branch(ctx, true_branch, false_branch);
    }

    void Or::gen_branch(CodegenContext &ctx, std::string true_branch, std::string false_branch) {
        std::string right_part = ctx.new_branch_label("or");
        left_.gen_branch(ctx, right_part, true_branch);
        ctx.emit(right_part + ": ;");
        right_.gen_branch(ctx, true_branch, false_branch);
    }

    void Not::gen_branch(CodegenContext &ctx, std::string true_branch, std::string false_branch) {
        left_.gen_branch(ctx, false_branch, true_branch);
    }

    void AsBool::gen_branch(CodegenContext &ctx, std::string true_branch, std::string false_branch) {
        // At present, we don't have 'and' and 'or'
        std::string reg = ctx.alloc_reg();
        left_.gen_rvalue(ctx, reg);
        ctx.emit(std::string("if (") + reg + ") goto " + true_branch + ";");
        ctx.emit(std::string("goto ") + false_branch + ";");
        ctx.free_reg(reg);
    }

    /* In the Quack AST, there is a separate "Load" node that
     * converts an lexpr to an rexpr.  In the calculator there
     * is no separate "Load" node, so I need both gen_lvalue
     * and gen_rvalue for an Ident.
     * Note this may generate a declaration in the C code,
     * so it should be called *between* complete C statements.
     */
    std::string Ident::gen_lvalue(CodegenContext& ctx) {
        return ctx.get_local_var(text_);
    }

    /* For an r_value, we load the value of the variable into a
     * "register", which in our C code is just a temporary variable.
     */
    void Ident::gen_rvalue(CodegenContext &ctx, std::string target_reg) {
        /* The lvalue, i.e., address of memory */
        std::string loc = ctx.get_local_var(text_);
        /* In assembly language we would generate a LOAD instruction;
         * in C we generate an assignment.
         */
        ctx.emit(target_reg + " = " + loc + "; // LOAD");
    }

    /* Constants have rvalues but not lvalues, because you should
     * really not change the values of your constants.
     */
    void IntConst::gen_rvalue(CodegenContext &ctx, std::string target_reg) {
        ctx.emit(target_reg + " = " + std::to_string(value_)
            + "; // LOAD constant value");
    }

    /* Binary operators */

    void Plus::gen_rvalue(CodegenContext &ctx, std::string target_reg) {
        left_.gen_rvalue(ctx, target_reg);
        std::string right_reg = ctx.alloc_reg();
        right_.gen_rvalue(ctx, right_reg);
        ctx.emit(target_reg + " = (" + target_reg + ") + ("
             + right_reg + "); // Plus");
        ctx.free_reg(right_reg);
    }

    void Minus::gen_rvalue(CodegenContext &ctx, std::string target_reg) {
        left_.gen_rvalue(ctx, target_reg);
        std::string right_reg = ctx.alloc_reg();
        right_.gen_rvalue(ctx, right_reg);
        ctx.emit(target_reg + " = (" + target_reg + ") - ("
                 + right_reg + "); // Minus");
        ctx.free_reg(right_reg);
    }

    void Times::gen_rvalue(CodegenContext &ctx, std::string target_reg) {
        left_.gen_rvalue(ctx, target_reg);
        std::string right_reg = ctx.alloc_reg();
        right_.gen_rvalue(ctx, right_reg);
        ctx.emit(target_reg + " = (" + target_reg + ") * ("
                 + right_reg + "); // Times");
        ctx.free_reg(right_reg);
    }

    void Div::gen_rvalue(CodegenContext &ctx, std::string target_reg) {
        left_.gen_rvalue(ctx, target_reg);
        std::string right_reg = ctx.alloc_reg();
        right_.gen_rvalue(ctx, right_reg);
        ctx.emit(target_reg + " = (" + target_reg + ") / ("
                 + right_reg + "); // Div");
        ctx.free_reg(right_reg);
    }

    /* ========================================== */

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

    void Not::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Not", out, ctx);
        json_child("left_", left_, out, ctx);
        json_close(out, ctx);
    }

    void AsBool::json(std::ostream& out, AST_print_context& ctx) {
        json_head("AsBool", out, ctx);
        json_child("left_", left_, out, ctx);
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