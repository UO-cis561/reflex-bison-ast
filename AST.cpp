#include "AST.h"
namespace AST
{
    void ASTNode::json_indent(std::ostream& out, unsigned int indent){
        for (unsigned int i=0; i<indent; i++){
                out << "  ";
        }
    }
    void ASTNode::json_head(std::ostream& out, unsigned int &indent, std::string node_kind){
        this->json_indent(out, indent);
        out << "{" << std::endl;
        this->json_indent(out, indent+1);
        out << "\"kind\" : \"" << node_kind << "\"";
        indent++;
    }
    void ASTNode::json_close(std::ostream& out, unsigned int &indent){
        if (indent > 0) indent--;
        this->json_indent(out, indent);
        out << "}";
    }
    void ASTNode::json_child(std::ostream& out, unsigned int indent, std::string field, ASTNode& child, char sep){
        this->json_indent(out, indent);
        out << "\"" << field << "\" : ";
        child.json(out, indent);
        out << sep;
    }
    void Block::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Block");
        out << "," << std::endl;
        this->json_indent(out, indent);
        indent++;
        out << "\"stmts_\" : [";
        std::string sep = "";
        for (ASTNode *stmt: stmts_) {
                out << sep << std::endl;
                stmt->json(out, indent);
                sep = ",";
        }
        indent--;
        out << std::endl;
        this->json_indent(out, indent);
        out << "]" << std::endl;
        this->json_close(out, indent);
    }
    void Ident::json(std::ostream &out, unsigned int indent){
        json_head(out, indent, "Ident");
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"name_\" : \"" << this->name_ << "\"" << std::endl;
        json_close(out, indent);
    }
    void Class::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Class");
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"name_\" :" << std::endl;
        this->name_->json(out, indent);
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"args_\" :" << std::endl;
        this->args_->json(out, indent);
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"extends_\" :" << std::endl;
        this->extends_->json(out, indent);
        out << std::endl;
        this->json_close(out, indent);
    }
    void Arg::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Arg");
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"name_\" :" << std::endl;
        this->name_->json(out, indent+1);
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"type_\" :" << std::endl;
        this->type_->json(out, indent+1);
        out << std::endl;
        this->json_close(out, indent);
    }
    void Leaf::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Leaf");
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"name_\" : \"" << this->name_ << "\"" << std::endl;
        this->json_close(out, indent);
    }
}