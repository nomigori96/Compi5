#ifndef HW3SOL_SEMANTIC_TYPES_HPP
#define HW3SOL_SEMANTIC_TYPES_HPP

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include "bp.hpp"

using namespace std;

struct exp_t{
    string* type;
    string* varName;
    vector<pair<int, BranchLabelIndex >>* trueList;
    vector<pair<int, BranchLabelIndex >>* falseList;
};

struct type_t{
    string* type;
};

struct enumType_t{
    string* typeName;
};

struct call_t{
    string* type;
    string* varName;
};

struct expList_t{
    vector<string>* types;
    vector<string>* varNames;
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
    string* retType;
};

struct enumIdDecl_t{
    string* enumId;
};

struct markerM_t{
    string* labelToJump;
};

struct statement_t{
    vector<pair<int, BranchLabelIndex >>* nextList;
    bool isNextList;
    vector<pair<int, BranchLabelIndex >>* breakList;
    bool isBreakList;
};

struct ifHeader_t{
    vector<pair<int, BranchLabelIndex >>* falseList;
};

struct markerN_t{
    vector<pair<int, BranchLabelIndex >>* nextList;
};

struct whileHeader_t{
    vector<pair<int, BranchLabelIndex >>* falseList;
    string* whileExpLabel;
};

#endif //HW3SOL_SEMANTIC_TYPES_HPP
