#include "AST.h"
namespace AST
{
    void ASTNode::json_indent(std::ostream& out, unsigned int indent){
        for (unsigned int i=0; i<indent; i++){
                out << "  ";
        }
    }
    void ASTNode::json_head(std::ostream& out, unsigned int &indent, std::string node_kind){
        out << "{" << std::endl;
        indent++;
        this->json_indent(out, indent);
        out << "\"kind\": \"" << node_kind << "\"";
    }
    void ASTNode::json_close(std::ostream& out, unsigned int &indent){
        if (indent > 0) indent--;
        out << std::endl;
        this->json_indent(out, indent);
        out << "}";
    }
    void ASTNode::json_child(std::ostream& out, unsigned int indent, std::string field, ASTNode *child){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": ";
        child->json(out, indent);
    }
    void ASTNode::json_list(std::ostream& out, unsigned int indent, std::string field, std::vector<ASTNode*> &list){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": [";
        std::string sep = "";
        for (ASTNode *node: list) {
                out << sep << std::endl;
                this->json_indent(out, indent+1);
                node->json(out, indent+1);
                sep = ",";
        }
        if (list.size() > 0) {        
            out << std::endl;
            this->json_indent(out, indent);
        }
        out << "]";
    }
    void ASTNode::json_string(std::ostream& out, unsigned int indent, std::string field, std::string str){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": \"" << str << "\"";
    }
    void ASTNode::json_int(std::ostream& out, unsigned int indent, std::string field, unsigned int val){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": " << val;
    }
    void Block::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Block");
        this->json_list(out, indent, "stmts_", this->stmts_);
        this->json_close(out, indent);
    }
    void Ident::json(std::ostream &out, unsigned int indent){
        json_head(out, indent, "Ident");
        this->json_string(out, indent, "text_", this->text_);
        json_close(out, indent);
    }
    void Class::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Class");
        this->json_child(out, indent, "name_", this->name_);
        this->json_child(out, indent, "args_", this->args_);
        this->json_child(out, indent, "extends_", this->extends_);
        this->json_child(out, indent, "stmts_", this->stmts_);
        this->json_child(out, indent, "mthds_", this->mthds_);
        this->json_close(out, indent);
    }
    void Arg::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Arg");
        this->json_child(out, indent, "name_", this->name_);
        if (this->type_ != NULL)
            this->json_child(out, indent, "type_", this->type_);
        this->json_close(out, indent);
    }
    void Actuals::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Actuals");
        this->json_list(out, indent, "args_", this->args_);
        this->json_close(out, indent);
    }
    void LExpr::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "LExpr");
        if (this->obj_ != NULL) {       
            this->json_child(out, indent, "obj_", this->obj_);
        }
        this->json_child(out, indent, "name_", this->name_);
        this->json_close(out, indent);
    }
    void Arguments::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Arguments");
        this->json_list(out, indent, "args_", this->args_);
        this->json_close(out, indent);
    }
    void Call::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Call");
        this->json_child(out, indent, "obj_",  this->obj_);
        this->json_child(out, indent, "mthd_", this->mthd_);
        this->json_child(out, indent, "args_", this->args_);
        this->json_close(out, indent);
    }
    void Constructor::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Constructor");
        this->json_child(out, indent, "name_", this->name_);
        this->json_child(out, indent, "args_", this->args_);
        this->json_close(out, indent);
    }
    void And::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "AND");
        this->json_child(out, indent, "lhs_", this->lhs_);
        this->json_child(out, indent, "rhs_", this->rhs_);
        this->json_close(out, indent);
    }
    void Or::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "OR");
        this->json_child(out, indent, "lhs_", this->lhs_);
        this->json_child(out, indent, "rhs_", this->rhs_);
        this->json_close(out, indent);
    }
    void Not::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "NOT");
        this->json_child(out, indent, "expr_", this->expr_);
        this->json_close(out, indent);
    }
    void Assignment::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Assignment");
        this->json_child(out, indent, "l_expr_", this->l_expr_);
        if (this->type_ != NULL) {
            this->json_child(out, indent, "type_", this->type_);
        }
        this->json_child(out, indent, "r_expr_", this->r_expr_);
        this->json_close(out, indent);
    }
    void IntLit::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "IntLit");
        this->json_int(out, indent, "val_", this->val_);
        this->json_close(out, indent);
    }
    void StrLit::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "StrLit");
        this->json_string(out, indent, "text_", this->text_);
        this->json_close(out, indent);
    }
    void Return::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Return");
        if (this->r_expr_ != NULL) {
            this->json_child(out, indent, "r_expr_", this->r_expr_);
        }
        this->json_close(out, indent);
    }
    void Method::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Method");
        this->json_child(out, indent, "name_", this->name_);
        this->json_child(out, indent, "args_", this->args_);
        if (this->type_ != NULL){
            this->json_child(out, indent, "type_", this->type_);
        }
        this->json_child(out, indent, "stmts_", this->stmts_);
        this->json_close(out, indent);
    }
    void If::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "If");
        this->json_child(out, indent, "cond_",       this->cond_);
        this->json_child(out, indent, "if_stmts_",   this->if_stmts_);
        this->json_child(out, indent, "else_stmts_", this->else_stmts_);
        this->json_close(out, indent);
    }
    void While::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "While");
        this->json_child(out, indent, "cond_",       this->cond_);
        this->json_child(out, indent, "stmts_",   this->stmts_);
        this->json_close(out, indent);
    }
    void TypeAlt::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "TypeAlt");
        this->json_child(out, indent, "arg_",   this->arg_);
        this->json_child(out, indent, "stmts_", this->stmts_);
        this->json_close(out, indent);
    }
    void Typecase::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Typecase");
        this->json_child(out, indent, "expr_", this->expr_);
        this->json_list(out, indent, "stmts_", this->stmts_);
        this->json_close(out, indent);
    }
}