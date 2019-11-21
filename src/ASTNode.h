//
// Created by Michal Young on 9/12/18.
//

#ifndef REFLEXIVE_ASTNODE_H
#define REFLEXIVE_ASTNODE_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <assert.h>
#include "CodegenContext.h"
#include "EvalContext.h"

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Json conversion and pretty-printing can pass around a print context object
    // to keep track of indentation, and possibly other things.
    class AST_print_context {
    public:
        int indent_; // Number of spaces to place on left, after each newline
        AST_print_context() : indent_{0} {};
        void indent() { ++indent_; }
        void dedent() { --indent_; }
    };


    class ASTNode {
    public:
        virtual int eval(EvalContext &ctx) = 0;        // Immediate evaluation

        /* Code generation: Of an lvalue, of an rvalue, and of a branch */
        /* Each subtree may implement some of these and not others, so the default
         * implementations are code-generation errors.
         */
        virtual void gen_rvalue(CodegenContext& ctx, std::string target_reg) {
            std::cerr << "*** No rvalue for this node ***" << std::endl;
            assert(false);  // Invoke the debugger!
        }
        /* For the calculator, an lvalue will be the name of a local
         * variable in the generated C code.  In assembly language,
         * it would be a register holding the memory address of the
         * location, which might be calculated in various ways such
         * as adding an offset to the beginning of an array.
         */
        virtual std::string gen_lvalue(CodegenContext& ctx) {
            std::cerr << "*** No lvalue for this node ***" << std::endl;
            assert(false);
        }
        virtual void gen_branch(CodegenContext& ctx, std::string true_branch, std::string false_branch) {
            std::cerr << "*** No branching on this node ****" << std::endl;
            assert(false);
        }

        /* Dump JSON representation */
        virtual void json(std::ostream& out, AST_print_context& ctx) = 0;

        std::string str() {  // String representation is JSON
            std::stringstream ss;
            AST_print_context ctx;
            json(ss, ctx);
            return ss.str();
        }
    protected:
        void json_indent(std::ostream& out, AST_print_context& ctx);
        void json_head(std::string node_kind, std::ostream& out, AST_print_context& ctx);
        void json_close(std::ostream& out, AST_print_context& ctx);
        void json_child(std::string field, ASTNode& child, std::ostream& out, AST_print_context& ctx, char sep=',');
    };

    /* A block is a sequence of statements or expressions.
     * For simplicity we'll just make it a sequence of ASTNode,
     * and leave it to the parser to build valid structures.
     */
    class Block : public ASTNode {
        std::vector<ASTNode*> stmts_;
    public:
        explicit Block() : stmts_{std::vector<ASTNode*>()} {}
        void append(ASTNode* stmt) { stmts_.push_back(stmt); }
        int eval(EvalContext& ctx) override;
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        void json(std::ostream& out, AST_print_context& ctx) override;
     };

    /* L_Expr nodes are AST nodes that can be evaluated for location.
     * Most can also be evaluated for value_.  An example of an L_Expr
     * is an identifier, which can appear on the left_ hand or right_ hand
     * side of an assignment.  For example, in x = y, x is evaluated for
     * location and y is evaluated for value_.
     *
     * For now, a location is just a name, because that's what we index
     * the symbol table with.  In a full compiler, locations can be
     * more complex, and typically in code generation we would have
     * LExpr evaluate to an address in a register.
     *
     * LExpr is abstract.  It's only concrete subclass for now is Ident,
     * but in a full OO language we would have LExprs that look like
     * a.b and a[2].
     */
    class LExpr : public ASTNode {
    public:
        virtual std::string l_eval(EvalContext& ctx) = 0;
    };

    /* An assignment has an lvalue (location to be assigned to)
     * and an expression.  We evaluate the expression and place
     * the value_ in the variable.
     */

    class Assign : public ASTNode {
        LExpr &lexpr_;
        ASTNode &rexpr_;
    public:
        Assign(LExpr &lexpr, ASTNode &rexpr) :
           lexpr_{lexpr}, rexpr_{rexpr} {};
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        void json(std::ostream& out, AST_print_context& ctx) override;
        int eval(EvalContext& ctx) override;
        void r_eval(CodegenContext& ctx, std::string target_reg);
    };

    class If : public ASTNode {
        ASTNode &cond_; // The boolean expression to be evaluated
        Block &truepart_; // Execute this block if the condition is true
        Block &falsepart_; // Execute this block if the condition is false
    public:
        explicit If(ASTNode &cond, Block &truepart, Block &falsepart) :
            cond_{cond}, truepart_{truepart}, falsepart_{falsepart} { };
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        void json(std::ostream& out, AST_print_context& ctx) override;
        int eval(EvalContext& ctx) override;
    };

    /* We need a node to represent interpretation of an r-expression
     * as a boolean.  While r-expressions have a gen_rvalue method,
     * we need a gen_branch method for r-expressions that are
     * interpreted as booleans.
     */
    class AsBool : public ASTNode {
        ASTNode &left_;
    public:
        explicit AsBool(ASTNode &left) : left_{left} {}
        void gen_branch(CodegenContext& ctx,
                std::string true_branch, std::string false_branch) override;
        void json(std::ostream& out, AST_print_context& ctx) override;
        int eval(EvalContext& ctx) override;
    };

    /* Identifiers like x and literals like 42 are the
     * leaves of the AST.  A literal can only be evaluated
     * for value_ (the 'eval' method), but an identifier
     * can also be evaluated for location (when we want to
     * store something in it).
     */
    class Ident : public LExpr {
        std::string text_;
    public:
        explicit Ident(std::string txt) : text_{txt} {}
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        std::string gen_lvalue(CodegenContext& ctx) override;
        void json(std::ostream& out, AST_print_context& ctx) override;
        int eval(EvalContext &ctx) override;
        std::string l_eval(EvalContext& ctx) override { return text_; }
    };

    class IntConst : public ASTNode {
        int value_;
    public:
        explicit IntConst(int v) : value_{v} {}
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        void json(std::ostream& out, AST_print_context& ctx) override;
        int eval(EvalContext &ctx) override { return value_; }
    };

    // Virtual base class for +, -, *, /, etc
    class BinOp : public ASTNode {
        // each subclass must override the inherited
        // eval() method

    protected:
        std::string opsym;
        ASTNode &left_;
        ASTNode &right_;
        BinOp(std::string sym, ASTNode &l, ASTNode &r) :
                opsym{sym}, left_{l}, right_{r} {};
    public:
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class Plus : public BinOp {
    public:
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        int eval(EvalContext& ctx) override;
        Plus(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Plus"),  l, r) {};
    };

    class Minus : public BinOp {
    public:
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        int eval(EvalContext& ctx) override;
        Minus(ASTNode &l, ASTNode &r) :
            BinOp(std::string("Minus"),  l, r) {};
    };

    class Times : public BinOp {
    public:
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        int eval(EvalContext& ctx) override;
        Times(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Times"),  l, r) {};
    };

    class Div : public BinOp {
    public:
        void gen_rvalue(CodegenContext& ctx, std::string target_reg) override;
        int eval(EvalContext& ctx) override;
        Div (ASTNode &l, ASTNode &r) :
                BinOp(std::string("Div"),  l, r) {};
    };


}
#endif //REFLEXIVE_ASTNODE_H
