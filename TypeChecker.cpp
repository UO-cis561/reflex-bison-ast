#include "TypeChecker.h"
#include "AST.h"
#include <map>

bool TypeChecker::CheckClasses() {
    if (!this->driver_->is_parsed()) {
        return false;
    }
    AST::Block *classes = this->driver_->get_root()->get_classes();
    
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
            class_names[name] = new Class_Struct(name);
        }
        if (class_names.find(super) == class_names.end()) {
            class_names[super] = new Class_Struct(super);
        }
        class_names[name]->super = class_names[super];
    }
    for (std::pair<std::string, Class_Struct*> node : class_names){
        if (node.second->super == NULL && node.second->noCycles == false) {
            //Parent class has no super but is not Obj
            std::cerr << "Error: Class \"" << node.first << "\" was not defined" << std::endl;
            return false;
        }
    }
    for (std::pair<std::string, Class_Struct*> node : class_names){
        Class_Struct *curr = node.second;
        if (curr->noCycles)
            continue;
        Class_Struct *curr2 = node.second->super;
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
        Class_Struct *curr = it->second;
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

bool TypeChecker::CheckMethodsAndTypes() {
    if (!checked_classes) {
        return false;
    }
    AST::Block *classes = this->driver_->get_root()->get_classes();
    //std::map<std::string, std::string> symbol_table;
    bool changed = false;
    bool first_loop = true;
    std::map<std::string, std::string> symbol_table;
    do{
        changed = false;
        for (size_t i=0; i<classes->size(); i++) { 
            AST::Class *curr_class = (AST::Class*)((*classes)[i]);
            std::string class_name = curr_class->get_name();
            std::vector<std::string> declared_in_class;
            Class_Struct *class_node = class_names[class_name];
            class_node->symbol_table["this"] = class_name;
            declared_in_class.push_back("this");
            declared_in_class.push_back("true");
            declared_in_class.push_back("false");
            declared_in_class.push_back("none");
            
            for (std::pair<std::string, std::string> arg : curr_class->get_args()){
                class_node->symbol_table[arg.first] = arg.second;
                declared_in_class.push_back(arg.first);
            }
            for (AST::Statement *stmt : curr_class->get_stmts()) {
                stmt->append_symbol_table(class_node->symbol_table, declared_in_class, changed, class_names, LCA);
            }
            AST::Block *mthds = curr_class->get_mthds();
            for (size_t j=0; j<mthds->size(); j++){
                AST::Method *mthd = (AST::Method*)((*mthds)[j]);
                std::string name = mthd->get_name();
                if (class_names.find(name) != class_names.end()){
                    std::cerr << "Error: Class and Method both share identifier \"" << name << "\"" << std::endl;
                    return false;
                }
                std::vector<std::pair<std::string,std::string>> args = mthd->get_args();
                std::vector<std::string> argTypes(args.size());
                for (size_t i=0; i<args.size(); i++) {
                    std::string arg_name = args[i].first;
                    std::string arg_type = args[i].second;
                    if (class_names.find(arg_type) == class_names.end()) {
                        std::cerr << "Error: Argument \"" << arg_name << "\" in method \"" << curr_class->get_name() << "." << name << "()\" has unknown type \"" << arg_type << "\"" << std::endl;
                        return false;
                    }
                    argTypes[i] = arg_type;
                }

                std::pair<std::string, std::vector<std::string>> mthd_key(name, argTypes);
                if (first_loop && class_node->methods.find(mthd_key) != class_node->methods.end()){
                    std::cerr << "Error: Redefinition of method \"" << name << "(";
                    std::string sep = "";
                    for (std::string type : argTypes){
                        std::cerr << sep << type;
                        sep = ", ";
                    }
                    std::cerr << ")\"" << std::endl;
                    return false;
                }
                Method_Struct *mthd_struct;
                if (first_loop){
                    mthd_struct = new Method_Struct();
                    mthd_struct->name = name;
                    mthd_struct->clazz = class_name;
                    mthd_struct->args = args;
                    std::string explicit_type = mthd->get_type();
                    if (explicit_type != "") {
                        if (class_names.find(explicit_type) != class_names.end()){
                            mthd_struct->symbol_table["return"] = explicit_type;
                        }else{
                            std::cerr << "Error: Method \"" << name << "\" returns unknown type \"" << explicit_type << "\"" << std::endl;
                            return false;
                        }
                    }
                    class_node->methods[mthd_key] = mthd_struct;
                }else{
                    mthd_struct = class_node->methods[mthd_key];
                }

                std::vector<AST::Statement*> stmts = mthd->get_stmts();

                std::vector<std::string> declared_in_args(args.size() + 1);
                declared_in_args[0] = "this";
                mthd_struct->symbol_table["this"] = class_name;
                for (size_t i=0; i<args.size(); i++) {
                    std::string arg_name = args[i].first;
                    declared_in_args[i+1] = arg_name;
                    mthd_struct->symbol_table[args[i].first] = args[i].second;
                }
                std::vector<std::string> declared;
                declared.reserve(declared_in_class.size() + declared_in_args.size());
                declared.insert(declared.end(), declared_in_class.begin(), declared_in_class.end());
                declared.insert(declared.end(), declared_in_args.begin(), declared_in_args.end());

                for (AST::Statement *stmt : stmts) {
                    stmt->append_symbol_table(mthd_struct->symbol_table, declared, changed, class_names, LCA);
                }
            }
        }
        AST::Block *stmts =  this->driver_->get_root()->get_stmts();
        std::vector<std::string> declared;
        declared.push_back("true");
        declared.push_back("false");
        declared.push_back("none");
        for (size_t i=0; i<stmts->size(); i++){
            AST::Statement *stmt = (AST::Statement*)((*stmts)[i]);
            stmt->append_symbol_table(symbol_table, declared, changed, class_names, LCA);
        }
        first_loop = false;
    }while(changed);
    do{
        changed = false;
        for (name_to_class_map::iterator class_it = class_names.begin(); class_it != class_names.end(); class_it++){
            Class_Struct *curr = class_it->second;
            if (curr->super != NULL){
                Class_Struct *super = curr->super;
                for (std::map<std::string, std::string>::iterator symbol_it = super->symbol_table.begin(); symbol_it != super->symbol_table.end(); symbol_it++){
                    std::string name = symbol_it->first;
                    std::string super_type = symbol_it->second;
                    if (curr->symbol_table.find(name) == curr->symbol_table.end()){
                        curr->symbol_table[name] = super_type;
                        changed = true;
                    }else{
                        std::string derived_type = curr->symbol_table[name];
                        if (class_names.find(derived_type) != class_names.end() && class_names.find(super_type) != class_names.end()){
                            std::string ancestor = LCA[derived_type][super_type];
                            if (ancestor != super_type){
                                std::cerr << "Error: Class \"" << curr->name << "\" has a field \"" << name << ":" << derived_type 
                                        << "\" that is not a sub-type of its parent's field \"" << name << ":" << super_type << "\"" << std::endl;
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }while(changed);
    std::cout << "-----Classes------" << std::endl;
    for (std::map<std::string, Class_Struct*>::iterator it = class_names.begin(); it != class_names.end(); it++) {
        std::cout << "class " << it->second->name;
        if (it->second->super != NULL && it->second->super->name != "Obj"){
            std::cout << " extends " << it->second->super->name;
        }
        std::cout << std::endl << "\tSymbols:" << std::endl;
        for (std::map<std::string, std::string>::iterator it2 = it->second->symbol_table.begin(); it2 != it->second->symbol_table.end(); it2++) {
            std::cout << "\t\t" << it2->first << ": " << it2->second << std::endl;
        }
        std::cout << "\tMethods:" << std::endl;
        for (std::map<std::pair<std::string, std::vector<std::string>>, struct Method_Struct*>::iterator it2 = it->second->methods.begin(); it2!=it->second->methods.end(); it2++){
            std::cout << "\t\t" << it2->second->name << "(";
            std::string sep = "";
            for (std::pair<std::string, std::string> arg : it2->second->args){
                std::cout << sep << arg.first << ": " << arg.second;
                sep = ", ";
            }
            std::cout << ")";
            if (it2->second->symbol_table["return"] != "") {
                std::cout << " -> " << it2->second->symbol_table["return"];
            }
            std::cout << std::endl;
            for (std::map<std::string, std::string>::iterator it3 = it2->second->symbol_table.begin(); it3 != it2->second->symbol_table.end(); it3++) {
                std::cout << "\t\t\t" << it3->first << ": " << it3->second << std::endl;
            }
        }
    }
    std::cout << "---Symbol Table---" << std::endl;
    for (std::map<std::string, std::string>::iterator it = symbol_table.begin(); it != symbol_table.end(); it++) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "------------------" << std::endl;
    return true;
}