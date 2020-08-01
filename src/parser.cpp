//
// The parser driver just glues together a parser object
// and a lexer object.
//

#include "lex.yy.h"
#include "ASTNode.h"
#include "EvalContext.h"
#include "Messages.h"
#include <unistd.h>
#include <iostream>

class Driver {
public:
    explicit Driver(const reflex::Input in) : lexer(in), parser(new yy::parser(lexer, &root))
       { root = nullptr; }
    ~Driver() { delete parser; }
    AST::ASTNode* parse() {
        // parser->set_debug_level(1); // 0 = no debugging, 1 = full tracing
        // std::cout << "Running parser\n";
        int result = parser->parse();
        if (result == 0 && report::ok()) {  // 0 == success, 1 == failure
            // std::cout << "Extracting result\n";
            if (root == nullptr) {
                std::cout << "But I got a null result!  How?!\n";
            }
            return root;
        } else {
            std::cout << "Parse failed, no tree\n";
            return nullptr;
        }
    }
private:
    yy::Lexer   lexer;
    yy::parser *parser;
    AST::ASTNode *root;
};

void generate_code(AST::ASTNode *root) {
    CodegenContext ctx(std::cout);
    // Prologue
    ctx.emit("#include <stdio.h>");
    ctx.emit("int main(int argc, char **argv) {");
    // Body of generated code
    std::string target = ctx.alloc_reg();
    root->gen_rvalue(ctx, target);
    // Coda
    ctx.emit(std::string(R"(printf("-> %d\n",)")
        + target + ");");
    ctx.emit("}");
}

int main(int argc, char **argv)
{
    AST::ASTNode* root;
    // std::istream *source;
    /* Choices of output */
    int json = 0;
    int codegen = 0;
    int calcmode = 0;
    char opt;
    while ((opt = getopt (argc, argv, "jce")) != -1) {
        if (opt == 'j') { json = 1; }
        if (opt == 'e') { calcmode = 1;}
        if (opt == 'c') { codegen = 1; }
    }
    // The remaining argument should be a file name
    if (optind < argc) {
        const char* path = argv[optind];
        std::cerr << "Reading from file " << path << std::endl;
        FILE *f = fopen(path, "r");
        if (! f) {
            std::cerr << "Open failed on '" << path << "'" << std::endl;
            exit(5);
        }
        std::cerr << "Opened " << argv[optind] << std::endl;
        Driver driver(f);
        root = driver.parse();
    } else {
        std::cerr << "Reading from stdin" << std::endl;
        Driver driver(&std::cin);
        root = driver.parse();
    }
    if (root != nullptr) {
        std::cerr << "Parsed!\n";
        if (json) {
            AST::AST_print_context context;
            root->json(std::cout, context);
            std::cout << std::endl;
        }
        if (calcmode) {
            auto ctx = EvalContext();
            std::cout << "Evaluates to " << root->eval(ctx) << std::endl;
            exit(0);
        }
        if (codegen) {
            std::cout << "/* BEGIN GENERATED CODE */" << std::endl;
            generate_code(root);
            std::cout << "/* END GENERATED CODE */" << std::endl;
        }
    } else {
        std::cerr << "Extracted root was nullptr" << std::endl;
        exit(1);
    }
}
