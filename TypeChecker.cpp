#include "TypeChecker.h"
#include "AST.h"
#include <map>



bool TypeChecker::CheckClasses() {
    if (!this->driver_->is_parsed()) {
        return false;
    }
    AST::Block *classes = this->driver_->get_root()->get_classes();
    class_names["Obj"] = new ClassTree("Obj");
    class_names["Obj"]->noCycles = true;
    class_names["Nothing"] = new ClassTree("Nothing");
    class_names["Nothing"]->noCycles = true;
    class_names["String"] = new ClassTree("String");
    class_names["String"]->noCycles = true;
    class_names["String"]->super = class_names["Obj"];
    class_names["Int"] = new ClassTree("Int");
    class_names["Int"]->noCycles = true;
    class_names["Int"]->super = class_names["Obj"];
    class_names["Boolean"] = new ClassTree("Boolean");
    class_names["Boolean"]->noCycles = true;
    class_names["Boolean"]->super = class_names["Obj"];
    
    for (size_t i=0; i<classes->size(); i++) {
        AST::Class *curr_class = (AST::Class*)((*classes)[i]);
        std::string name = curr_class->get_name();
        if (    name == "Obj" || 
                name == "Nothing" || 
                name == "String" || 
                name == "Int" ||
                name == "Boolean") {
            std::cerr << "Error: Redefinition of class \"" << name << "\"" << std::endl;
            return false;
        }
        std::string super = curr_class->get_extends();
        if (    super == "Nothing" || 
                super == "String" || 
                super == "Int" ||
                super == "Boolean") {
            std::cerr << "Error: Class \"" << name << "\" cannot extend \"" << super << "\"" << std::endl;
            return false;
        }
        if (class_names.find(name) != class_names.end()) {
            if (class_names[name]->super != NULL) {
                std::cerr << "Error: Redefinition of class \"" << name << "\"" << std::endl;
                return false;
            }
        }else{
            class_names[name] = new ClassTree(name);
        }
        if (class_names.find(super) == class_names.end()) {
            class_names[super] = new ClassTree(super);
        }
        class_names[name]->super = class_names[super];
    }
    for (std::pair<std::string, ClassTree*> node : class_names){
        if (node.second->super == NULL && node.second->noCycles == false) {
            //Parent class has no super but is not Obj
            std::cerr << "Error: Class \"" << node.first << "\" was not defined" << std::endl;
            return false;
        }
    }
    for (std::pair<std::string, ClassTree*> node : class_names){
        ClassTree *curr = node.second;
        if (curr->noCycles)
            continue;
        ClassTree *curr2 = node.second->super;
        while(!curr->noCycles && !curr2->noCycles){
            if (curr == curr2) {
                std::cerr << "Error: Class hierarchy contains a cycle involving class \"" << curr->name << "\"" << std::endl;
                return false;
            }
            
            curr = curr->super;
            curr2 = curr2->super->super;
        }
        
        curr = node.second;
        while (!curr->noCycles) {
            curr->noCycles = true;
            curr = curr->super;
        }
    }
    checked_classes = true;
    return true;
}

void TypeChecker::FindLCAs() {
    if (!checked_classes) {
        return;
    }
    //Iterate through each class name to find all of its common ancestors
    for (name_to_class_map::iterator it = class_names.begin(); it != class_names.end(); it++) {
        ClassTree *curr = it->second;
        //Climb up through the current class's ancestors, adding them to its LCAs.
        while (curr != NULL) {
            LCA[it->first][curr->name] = curr->name;
            LCA[curr->name][it->first] = curr->name;
            //Find any LCAs of the current ancestor not already in the current class's LCAs and add them.
            for (std::map<std::string, std::string>::iterator it2 = LCA[curr->name].begin(); it2 != LCA[curr->name].end(); it2++) {
                if (LCA[it->first].find(it2->first) == LCA[it->first].end()) {
                    LCA[it->first][it2->first] = it2->second;
                    LCA[it2->first][it->first] = it2->second;
                }
            }
            curr = curr->super;
        }
    }
}

bool TypeChecker::CheckMethods() {
    if (!checked_classes) {
        return false;
    }
    AST::Block *classes = this->driver_->get_root()->get_classes();
    std::map<std::string, std::string> symbol_table;
    bool changed = false;
    do{
        changed = false;
        for (size_t i=0; i<classes->size(); i++) { 
            AST::Class *curr_class = (AST::Class*)((*classes)[i]);
            std::string class_name = curr_class->get_name();
            std::vector<std::string> declared_in_class;
            symbol_table["this"] = class_name;
            declared_in_class.push_back("this");
            for (AST::Statement *stmt : curr_class->get_stmts()) {
                stmt->append_symbol_table(symbol_table, declared_in_class, changed, class_names, LCA, mthd_return_types);
            }
            AST::Block *mthds = curr_class->get_mthds();
            for (size_t j=0; j<mthds->size(); j++){
                AST::Method *mthd = (AST::Method*)((*mthds)[j]);
                std::string name = mthd->get_name();
                std::vector<AST::Arg*> args = mthd->get_args();
                std::vector<std::string> argTypes(args.size()+1);
                argTypes[0] = class_name;
                for (size_t i=0; i<args.size(); i++) {
                    std::string arg_name = args[i]->get_name();
                    std::string arg_type = args[i]->get_type();
                    if (class_names.find(arg_type) == class_names.end()) {
                        std::cerr << "Error: Argument \"" << arg_name << "\" in method \"" << curr_class->get_name() << "." << name << "()\" has unknown type \"" << arg_type << "\"" << std::endl;
                        return false;
                    }
                    argTypes[i+1] = arg_type;
                }
                std::pair<std::string, std::vector<std::string>> mthd_key(name, argTypes);
                /*
                if (mthd_return_types.find(mthd_key) != mthd_return_types.end()) {
                    std::cerr << "Error: Redefinition of method \"" << name << "(";
                    std::string sep = "";
                    for (std::string type : argTypes){
                        std::cerr << sep << type;
                        sep = ", ";
                    }
                    std::cerr << ")\"" << std::endl;
                    return false;
                }
                 */
                std::vector<AST::Statement*> stmts = mthd->get_stmts();

                std::vector<std::string> declared_in_args(args.size() + 1);
                declared_in_args[0] = "this";
                for (size_t i=0; i<args.size(); i++) {
                    std::string arg_name = args[i]->get_name();
                    declared_in_args[i+1] = arg_name;
                    symbol_table[args[i]->get_name()] = args[i]->get_type();
                }
                std::vector<std::string> declared;
                declared.reserve(declared_in_class.size() + declared_in_args.size());
                declared.insert(declared.end(), declared_in_class.begin(), declared_in_class.end());
                declared.insert(declared.end(), declared_in_args.begin(), declared_in_args.end());
                if (mthd_return_types.find(mthd_key) != mthd_return_types.end()){
                    symbol_table["return"] = mthd_return_types[mthd_key];
                }
                for (AST::Statement *stmt : stmts) {
                    stmt->append_symbol_table(symbol_table, declared, changed, class_names, LCA, mthd_return_types);
                }
                if (symbol_table.find("return") != symbol_table.end()) {
                    mthd_return_types[mthd_key] = symbol_table["return"];
                }
                symbol_table.erase("return");
                std::cout << name << "(";
                std::string sep = "";
                for (std::string type : argTypes){
                    std::cout << sep << type;
                    sep = ", ";
                }
                std::cout << ")" << std::endl;
                std::cout << "---Symbol Table---" << std::endl;
                for (std::map<std::string, std::string>::iterator it = symbol_table.begin(); it != symbol_table.end(); it++) {
                    std::cout << it->first << "\t" << it->second << std::endl;
                }
                std::cout << "------------------" << std::endl;
                std::cout << "---Method Table---" << std::endl;
                for (std::map<std::pair<std::string,std::vector<std::string>>, std::string>::iterator it = mthd_return_types.begin(); it != mthd_return_types.end(); it++) {
                    std::cout << it->first.first << "(";
                    std::string sep = "";
                    for (std::string arg : it->first.second) {
                        std::cout << sep << arg;
                        sep = ", ";
                    }
                    std::cout << ")\t" << it->second << std::endl;
                }
                std::cout << "------------------" << std::endl;
            }
        }
    }while(changed);
    std::cout << "---Symbol Table---" << std::endl;
    for (std::map<std::string, std::string>::iterator it = symbol_table.begin(); it != symbol_table.end(); it++) {
        std::cout << it->first << "\t" << it->second << std::endl;
    }
    std::cout << "------------------" << std::endl;
    std::cout << "---Method Table---" << std::endl;
    for (std::map<std::pair<std::string,std::vector<std::string>>, std::string>::iterator it = mthd_return_types.begin(); it != mthd_return_types.end(); it++) {
        std::cout << it->first.first << "(";
        std::string sep = "";
        for (std::string arg : it->first.second) {
            std::cout << sep << arg;
            sep = ", ";
        }
        std::cout << ")\t" << it->second << std::endl;
    }
    std::cout << "------------------" << std::endl;
    AST::Block *stmts =  this->driver_->get_root()->get_stmts();
    for (size_t i=0; i<stmts->size(); i++){
        AST::Statement *stmt = (AST::Statement*)((*stmts)[i]);
    }
    return true;
}