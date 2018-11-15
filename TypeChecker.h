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
    method_to_type_map mthd_return_types;
public:
    TypeChecker(Driver *driver): driver_(driver) {
        std::vector<std::string> PLUS_Int_Int_args(2, "Int");
        std::pair<std::string, std::vector<std::string>> PLUS_Int_Int_ident("PLUS", PLUS_Int_Int_args);
        mthd_return_types[PLUS_Int_Int_ident] = "Int";
    }
    bool CheckClasses();
    void FindLCAs();
    bool CheckMethods();
};

#endif /* TYPECHECKER_H */

