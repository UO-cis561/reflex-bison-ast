#ifndef AST_H
#define AST_H

#include <string>
#include <sstream>
#include <vector>

namespace AST
{
    class ASTNode {
    public:
            virtual void json(std::ostream &out, unsigned int indent = 0) = 0;
    protected:
        void json_indent(std::ostream& out, unsigned int indent);
        void json_head  (std::ostream& out, unsigned int &indent, std::string node_kind);
        void json_close (std::ostream& out, unsigned int &indent);
        void json_child (std::ostream& out, unsigned int indent, std::string field, ASTNode *child);
        void json_list  (std::ostream& out, unsigned int indent, std::string field, std::vector<ASTNode*> &list);
        void json_string(std::ostream& out, unsigned int indent, std::string field, std::string str);
        void json_int   (std::ostream& out, unsigned int indent, std::string field, unsigned int val);
    };

    class Block : public ASTNode {
        std::vector<ASTNode*> stmts_;
    public:
        void append(ASTNode *stmt) { stmts_.push_back(stmt); }
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Ident : public ASTNode {
        std::string text_;
    public:
        Ident(std::string name): text_{name} {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Arg : public ASTNode {
        Ident *name_;
        Ident *type_;
    public:
        Arg(Ident *name, Ident *type): name_(name), type_(type) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Actuals : public ASTNode {
        std::vector<ASTNode*> args_; // Only allow appending Arg*
    public:
        void append(Arg *arg) { args_.push_back(arg); }
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Class : public ASTNode {
        Ident   *name_;
        Actuals *args_;
        Ident   *extends_;
        Block   *stmts_;
        Block   *mthds_;
    public:
        Class(Ident *name, Actuals *args, Ident *extends, Block *stmts, Block *mthds): name_(name), args_(args), extends_(extends), stmts_(stmts), mthds_(mthds) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class RExpr : public ASTNode {
    };

    class IntLit : public RExpr {
        unsigned int val_;
    public:
        IntLit(unsigned int val): val_(val) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class StrLit : public RExpr {
        std::string text_;
    public:
        StrLit(std::string text): text_(text) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class LExpr : public RExpr {
        RExpr *obj_;
        Ident *name_;
    public:
        LExpr(Ident *name): obj_(NULL), name_(name) {}
        LExpr(RExpr *obj, Ident *name): obj_(obj), name_(name) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Arguments : public ASTNode {
        std::vector<ASTNode*> args_; // Only allow appending RExpr*
    public:
        void append(RExpr *arg) { args_.push_back(arg); }
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Call : public RExpr {
        RExpr *obj_;
        Ident *mthd_;
        Arguments *args_;
    public:
        Call(RExpr *obj, Ident *mthd, Arguments *args): obj_(obj), mthd_(mthd), args_(args) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Constructor : public RExpr {
        Ident *name_;
        Arguments *args_;
    public:
        Constructor(Ident *name, Arguments *args): name_(name), args_(args) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class And : public RExpr {
        RExpr *lhs_;
        RExpr *rhs_;
    public:
        And(RExpr *lhs, RExpr *rhs): lhs_(lhs), rhs_(rhs) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Or : public RExpr {
        RExpr *lhs_;
        RExpr *rhs_;
    public:
        Or(RExpr *lhs, RExpr *rhs): lhs_(lhs), rhs_(rhs) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Not : public RExpr {
        RExpr *expr_;
    public:
        Not(RExpr *expr): expr_(expr) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Assignment : public ASTNode {
        LExpr *l_expr_;
        Ident *type_;
        RExpr *r_expr_;
    public:
        Assignment(LExpr *l_expr, Ident *type, RExpr *r_expr): l_expr_(l_expr), type_(type), r_expr_(r_expr) {} 
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Return : public ASTNode {
        RExpr *r_expr_;
    public:
        Return(RExpr *r_expr): r_expr_(r_expr) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Method : public ASTNode {
        Ident   *name_;
        Actuals *args_;
        Ident   *type_;
        Block   *stmts_;
    public:
        Method(Ident *name, Actuals *args, Ident *type, Block *stmts): name_(name), args_(args), type_(type), stmts_(stmts) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class If : public ASTNode {
        RExpr *cond_;
        Block *if_stmts_;
        Block *else_stmts_;
    public:
        If(RExpr *cond, Block *if_stmts, Block *else_stmts): cond_(cond), if_stmts_(if_stmts), else_stmts_(else_stmts) {}
        void set_else(Block *else_stmts){ this->else_stmts_ = else_stmts; }
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class While : public ASTNode {
        RExpr *cond_;
        Block *stmts_;
    public:
        While(RExpr *cond, Block *stmts): cond_(cond), stmts_(stmts) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class TypeAlt : public ASTNode {
        Arg *arg_;
        Block *stmts_;
    public:
        TypeAlt(Arg *arg, Block *stmts): arg_(arg), stmts_(stmts) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Typecase : public ASTNode {
        RExpr *expr_;
        std::vector<ASTNode*> stmts_; // Only allow appending TypeAlt*
    public:
        Typecase(RExpr *expr): expr_(expr) {}
        void append(TypeAlt *stmt) { stmts_.push_back(stmt); }
        void json(std::ostream &out, unsigned int indent = 0);
    };
}

#endif