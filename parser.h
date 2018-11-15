#ifndef PARSER_H
#define PARSER_H
#include "lex.yy.h"
#include "EvalContext.h"
#include "Messages.h"
#include "AST.h"

class Driver {
public:
    explicit Driver(reflex::Input in) : lexer(in), parser(new yy::parser(lexer, &root)){ root = NULL; parsed = false; }
    ~Driver() { delete parser; }
    void parse() {
        int result = parser->parse();
        this->parsed = (result == 0 && report::ok());
    }
    void json(std::ostream &out){
        root->json(out);
    }
    bool is_parsed(){ return this->parsed; }
    AST::Root *get_root(){ return this->root; }
private:
    yy::Lexer     lexer;
    yy::parser   *parser;
    AST::Root    *root;
    bool          parsed;
};

#endif /* PARSER_H */

