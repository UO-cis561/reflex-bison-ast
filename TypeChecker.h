#ifndef TYPECHECKER_H
#define TYPECHECKER_H
#include "parser.h"
#include <map>
#include <vector>

class TypeChecker {
    Driver *driver_;
    bool checked_classes = false;
    name_to_class_map class_names;
    LCA_table LCA;
    void add_builtin_type(std::string name, std::string super = ""){
        class_names[name] = new Class_Struct(name);
        class_names[name]->noCycles = true;
        if (super != "")
            class_names[name]->super = class_names[super];
    }
    void add_builtin_method(std::string clazz, std::string name, std::string argType, std::string retType){
        std::vector<std::string> argTypes;
        if (argType != "")
            argTypes.push_back(argType);
        std::vector<std::pair<std::string, std::string>> args;
        if (argType != "")
            args.push_back(std::pair<std::string, std::string>("other", argType));
        std::pair<std::string, std::vector<std::string>> ident(name, argTypes);
        Method_Struct *mthd = new Method_Struct();
        mthd->name = name;
        mthd->clazz = clazz;
        mthd->args = args;
        mthd->symbol_table["return"] = retType;
        class_names[clazz]->methods[ident] = mthd;
    }
public:
    TypeChecker(Driver *driver): driver_(driver) {
        add_builtin_type("Obj");
        add_builtin_type("Nothing");
        add_builtin_type("String", "Obj");
        add_builtin_type("Int", "Obj");
        add_builtin_type("Boolean", "Obj");      
        
        add_builtin_method("Obj", "STRING", "", "String");
        add_builtin_method("Obj", "PRINT", "", "Nothing");
        add_builtin_method("Obj", "EQUALS", "Obj", "Boolean");
        
        add_builtin_method("String", "STRING", "", "String");
        add_builtin_method("String", "PRINT", "", "Nothing");
        add_builtin_method("String", "EQUALS", "Obj", "Boolean");
        add_builtin_method("String", "LESS", "String", "Boolean");
        add_builtin_method("String", "PLUS", "String", "String");
        
        add_builtin_method("Boolean", "STRING", "", "String");
        add_builtin_method("Boolean", "PRINT", "", "Nothing");
        add_builtin_method("Boolean", "EQUALS", "Obj", "Boolean");
        
        add_builtin_method("Nothing", "STRING", "", "String");
        add_builtin_method("Nothing", "PRINT", "", "Nothing");
        add_builtin_method("Nothing", "EQUALS", "Obj", "Boolean");
        
        add_builtin_method("Int", "STRING", "", "String");
        add_builtin_method("Int", "PRINT", "", "Nothing");
        add_builtin_method("Int", "EQUALS", "Obj", "Boolean");
        add_builtin_method("Int", "LESS", "Int", "Boolean");
        add_builtin_method("Int", "PLUS", "Int", "Int");
        add_builtin_method("Int", "TIMES", "Int", "Int");
        add_builtin_method("Int", "DIVIDE", "Int", "Int");
        add_builtin_method("Int", "MINUS", "Int", "Int");
        
    }
    bool CheckClasses();
    void FindLCAs();
    bool CheckMethodsAndTypes();
};

#endif /* TYPECHECKER_H */

