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
            void json_child (std::ostream& out, unsigned int indent, std::string field, ASTNode& child, char sep=',');
	};

	class Block : public ASTNode {
            std::vector<ASTNode*> stmts_;
	public:
            void append(ASTNode *stmt) { stmts_.push_back(stmt); }
            void json(std::ostream &out, unsigned int indent = 0);
	};
        
        class Ident : public ASTNode {
            std::string name_;
        public:
            Ident(std::string name): name_{name} {}
            void json(std::ostream &out, unsigned int indent = 0);
        };
        
        class Class : public ASTNode {
            Ident *name_;
            Block *args_;
            Ident *extends_;
            Block *stmts_;
            Block *mthds_;
        public:
            Class(Ident *name, Block *args, Ident *extends): name_(name), args_(args), extends_(extends) {}
            void json(std::ostream &out, unsigned int indent = 0);
        };
        
        class Arg : public ASTNode {
            Ident *name_;
            Ident *type_;
        public:
            Arg(Ident *name, Ident *type): name_(name), type_(type) {}
            void json(std::ostream &out, unsigned int indent = 0);
        };

	class Leaf : public ASTNode {
            std::string name_;
	public:
            Leaf(std::string name = ""): name_(name) {}
            void json(std::ostream &out, unsigned int indent = 0);
	};
}

#endif