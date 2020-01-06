#ifndef HW3SOL_SEMANTIC_TYPES_HPP
#define HW3SOL_SEMANTIC_TYPES_HPP

#include <string>
#include <vector>
#include <utility>
#include <tuple>

using namespace std;

struct exp_t{
    string* type;
};

struct type_t{
    string* type;
};

struct enumType_t{
    string* typeName;
};

struct call_t{
    string* type;
};

struct expList_t{
    vector<string>* types;
};

struct dummy_t{
};

struct formals_t{
    vector<tuple<string, string, bool>>* args;
};

struct formalList_t{
    vector<tuple<string, string, bool>>* args;
};

struct formalDecl_t{
    string* type;
    string* argName;
    bool isEnumType;
};

struct retType_t{
    string* type;
};

struct enumeratorList_t{
    vector<string>* values;
};

struct enumerator_t{
    string* value;
};

struct funcIdDecl_t{
    string* funcId;
};

struct enumIdDecl_t{
    string* enumId;
};


#endif //HW3SOL_SEMANTIC_TYPES_HPP
