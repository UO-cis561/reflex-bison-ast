//
// The parser driver just glues together a parser object
// and a lexer object.
//

#include "lex.yy.h"
#include "EvalContext.h"
#include "Messages.h"
#include "AST.h"

class Driver {
public:
    explicit Driver(reflex::Input in) : lexer(in), parser(new yy::parser(lexer, &root)){ root = NULL; }
    ~Driver() { delete parser; }
    bool parse() {
        int result = parser->parse();
        return result == 0 && report::ok();
    }
    void json(std::ostream &out){
        root->json(out);
    }
private:
    yy::Lexer     lexer;
    yy::parser   *parser;
    AST::ASTNode *root;
};

int main()
{
    Driver driver(std::cin);
    bool parse_result = driver.parse();
    if (parse_result) {
        std::cout << "Finished parse with no errors" << std::endl;
        driver.json(std::cout);
    } else {
        std::cout << "Unable to parse!" << std::endl;
    }
}
