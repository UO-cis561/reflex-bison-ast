#ifndef AST_H
#define AST_H

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

struct ClassTree{
    std::string name;
    ClassTree *super = NULL;
    bool noCycles = false;
    ClassTree(std::string name_in): name(name_in){}
};

typedef std::map<std::string, ClassTree*> name_to_class_map;
typedef std::map<std::string, std::map<std::string, std::string>> LCA_table;
typedef std::pair<std::string, std::vector<std::string>> method_ident;
typedef std::map<method_ident, std::string> method_to_type_map;

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
        size_t size(){ return stmts_.size(); }
        ASTNode *operator[](int index){ return stmts_.at(index); }
    };
    
    class Root : public ASTNode {
        Block *classes_;
        Block *stmts_;
    public:
        Root(Block *classes, Block *stmts): classes_(classes), stmts_(stmts) {}
        Block *get_classes(){ return classes_; }
        Block *get_stmts(){ return stmts_; }
        void json(std::ostream &out, unsigned int indent = 0);
    };

    class Ident : public ASTNode {
        std::string text_;
    public:
        Ident(std::string name): text_{name} {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_text(){ return text_; }
    };

    class Arg : public ASTNode {
        Ident *name_;
        Ident *type_;
    public:
        Arg(Ident *name, Ident *type): name_(name), type_(type) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_name(){ return name_->get_text(); }
        std::string get_type(){ return type_->get_text(); }
    };

    class Actuals : public ASTNode {
        std::vector<ASTNode*> args_; // Only allow appending Arg*. Must be declared as ASTNode* for json printing.
    public:
        void append(Arg *arg) { args_.push_back(arg); }
        void json(std::ostream &out, unsigned int indent = 0);
        std::vector<Arg*> get_args() {
            std::vector<Arg*> vec(args_.size());
            for (size_t i=0; i<args_.size(); i++){
                vec[i] = (Arg*)(args_[i]);
            }
            return vec;
        }
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
        std::string get_name(){ return name_->get_text(); }
        std::string get_extends(){ return extends_->get_text(); }
        Block *get_stmts(){ return stmts_; }
        Block *get_mthds(){ return mthds_; }
    };
    
    class Statement : public ASTNode {
    public:
        virtual void append_symbol_table(
                std::map<std::string, std::string> &table, 
                std::vector<std::string> &declared, 
                name_to_class_map const& classes, 
                LCA_table const& LCA, 
                method_to_type_map const& methods) = 0;
    };
    
    class Method : public ASTNode {
        Ident   *name_;
        Actuals *args_;
        Ident   *type_;
        Block   *stmts_;
    public:
        Method(Ident *name, Actuals *args, Ident *type, Block *stmts): name_(name), args_(args), type_(type), stmts_(stmts) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_name(){ return name_->get_text(); }
        std::string get_type(){ return type_->get_text(); }
        std::vector<Arg*> get_args(){ return args_->get_args(); }
        std::vector<Statement*> get_stmts(){
            std::vector<Statement*> vec(stmts_->size());
            for (size_t i=0; i<stmts_->size(); i++){
                vec[i] = (Statement*)((*stmts_)[i]);
            }
            return vec;
        }
    };
    
    class TypeAlt : public ASTNode {
        Arg *arg_;
        Block *stmts_;
    public:
        TypeAlt(Arg *arg, Block *stmts): arg_(arg), stmts_(stmts) {}
        void json(std::ostream &out, unsigned int indent = 0);
    };
    
    class RExpr : public Statement {
    public:
        void append_symbol_table(
                std::map<std::string, std::string> &table, 
                std::vector<std::string> &declared, 
                name_to_class_map const& classes, 
                LCA_table const& LCA, 
                method_to_type_map const& methods){}
        virtual std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods) = 0;
    };
    
    class Arguments : public ASTNode {
        std::vector<ASTNode*> args_; // Only allow appending RExpr*
    public:
        void append(RExpr *arg);
        void json(std::ostream &out, unsigned int indent = 0);
        std::vector<std::string> get_arg_types(std::map<std::string, std::string> const& table, method_to_type_map const& methods) {
            std::vector<std::string> types;
            for (ASTNode *arg : args_) {
                types.push_back(((RExpr*)arg)->get_type(table, methods));
            }
            return types;
        }
    };
    
    class Assignment : public Statement {
        class LExpr *l_expr_;
        Ident *type_;
        class RExpr *r_expr_;
    public:
        Assignment(class LExpr *l_expr, Ident *type, class RExpr *r_expr): l_expr_(l_expr), type_(type), r_expr_(r_expr) {} 
        void json(std::ostream &out, unsigned int indent = 0);
        void append_symbol_table(
                std::map<std::string, std::string> &table, 
                std::vector<std::string> &declared, 
                name_to_class_map const& classes, 
                LCA_table const& LCA, 
                method_to_type_map const& methods);
    };
   
    
    class If : public Statement {
        RExpr *cond_;
        Block *if_stmts_;
        Block *else_stmts_;
    public:
        If(RExpr *cond, Block *if_stmts, Block *else_stmts): cond_(cond), if_stmts_(if_stmts), else_stmts_(else_stmts) {}
        void set_else(Block *else_stmts){ this->else_stmts_ = else_stmts; }
        void json(std::ostream &out, unsigned int indent = 0);
        void append_symbol_table(
                std::map<std::string, std::string> &table, 
                std::vector<std::string> &declared, 
                name_to_class_map const& classes, 
                LCA_table const& LCA, 
                method_to_type_map const& methods);
    };
    
    class While : public Statement {
        RExpr *cond_;
        Block *stmts_;
    public:
        While(RExpr *cond, Block *stmts): cond_(cond), stmts_(stmts) {}
        void json(std::ostream &out, unsigned int indent = 0);
        void append_symbol_table(
                std::map<std::string, std::string> &table, 
                std::vector<std::string> &declared, 
                name_to_class_map const& classes, 
                LCA_table const& LCA, 
                method_to_type_map const& methods);
    };

    class Return : public Statement {
        RExpr *r_expr_;
    public:
        Return(RExpr *r_expr): r_expr_(r_expr) {}
        void json(std::ostream &out, unsigned int indent = 0);
        void append_symbol_table(
                std::map<std::string, std::string> &table, 
                std::vector<std::string> &declared, 
                name_to_class_map const& classes, 
                LCA_table const& LCA, 
                method_to_type_map const& methods);
    };

    class Typecase : public Statement {
        RExpr *expr_;
        std::vector<ASTNode*> stmts_; // Only allow appending TypeAlt*
    public:
        Typecase(RExpr *expr): expr_(expr) {}
        void append(TypeAlt *stmt) { stmts_.push_back(stmt); }
        void json(std::ostream &out, unsigned int indent = 0);
        void append_symbol_table(
                std::map<std::string, std::string> &table, 
                std::vector<std::string> &declared, 
                name_to_class_map const& classes, 
                LCA_table const& LCA, 
                method_to_type_map const& methods);
    };
    
    class IntLit : public RExpr {
        unsigned int val_;
    public:
        IntLit(unsigned int val): val_(val) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods){ return "Int"; }
    };

    class StrLit : public RExpr {
        std::string text_;
    public:
        StrLit(std::string text): text_(text) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods){ return "String"; }
    };

    class LExpr : public RExpr {
        RExpr *obj_;
        Ident *name_;
    public:
        LExpr(Ident *name): obj_(NULL), name_(name) {}
        LExpr(RExpr *obj, Ident *name): obj_(obj), name_(name) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods) { 
            std::string ident = "";
            if (obj_ != NULL) {
                ident += obj_->get_type(table, methods) + ".";
            }
            ident += name_->get_text();
            if (table.find(ident) != table.end()){
                return table.at(ident);
            }else{
                return "Unknown0 " + ident;
            }
        }
        std::string get_ident(std::map<std::string, std::string> const& table, method_to_type_map const& methods) {
            std::string ident = "";
            if (obj_ != NULL){
                ident += obj_->get_type(table, methods) + ".";
            }
            ident += name_->get_text();
            return ident;
        }
    };

    class Call : public RExpr {
        RExpr *obj_;
        Ident *mthd_;
        Arguments *args_;
    public:
        Call(RExpr *obj, Ident *mthd, Arguments *args): obj_(obj), mthd_(mthd), args_(args) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods){ 
            std::string mthd_name = mthd_->get_text();
            std::vector<std::string> arg_types;
            arg_types.push_back(obj_->get_type(table, methods));
            for (std::string type : args_->get_arg_types(table, methods)){
                arg_types.push_back(type);
            }
            std::pair<std::string, std::vector<std::string>> mthd_ident(mthd_name, arg_types);
            if (methods.find(mthd_ident) != methods.end()) {
                return methods.at(mthd_ident);
            }else{
                std::string ret = "Unknown1 ";
                ret += mthd_name + "(";
                std::string sep = "";
                for (std::string arg : arg_types) {
                    ret += sep + arg;
                    sep = ", ";
                }
                ret += ")";
                return ret;
            }
        }
    };

    class Constructor : public RExpr {
        Ident *name_;
        Arguments *args_;
    public:
        Constructor(Ident *name, Arguments *args): name_(name), args_(args) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods){
            return name_->get_text();
        }
    };

    class And : public RExpr {
        RExpr *lhs_;
        RExpr *rhs_;
    public:
        And(RExpr *lhs, RExpr *rhs): lhs_(lhs), rhs_(rhs) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods){ return "Boolean"; }
    };

    class Or : public RExpr {
        RExpr *lhs_;
        RExpr *rhs_;
    public:
        Or(RExpr *lhs, RExpr *rhs): lhs_(lhs), rhs_(rhs) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods){ return "Boolean"; }
    };

    class Not : public RExpr {
        RExpr *expr_;
    public:
        Not(RExpr *expr): expr_(expr) {}
        void json(std::ostream &out, unsigned int indent = 0);
        std::string get_type(std::map<std::string, std::string> const& table, method_to_type_map const& methods){ return "Boolean"; }
    };
}

#endif