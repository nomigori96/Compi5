
#ifndef HW3_PARSERFUNCTIONNEW_HPP
#define HW3_PARSERFUNCTIONNEW_HPP



#include <algorithm>
#include "symbol_table.hpp"
#include "hw3_output.hpp"
#include "bp.hpp"


using namespace output;

void CheckMainExists();

void OpenNewScope();

void CloseCurrentScope();

vector<string> MapArgsToTypes(vector<tuple<string, string, bool>> fromRecord);


void AddFunctionSymbolIfNotExists(
        const string& symbol_name,
        const string& ret_type);

void UpdateCurrFunctionRetType(string& retType);

void AddFuncArgsToSymbolTable(vector<tuple<string,string,bool>>& args);

void UpdateFunctionSymbolArgs(vector<tuple<string,string,bool>>& args, string func_id);

void CheckIfIdIsShadowing(string& id);

void CheckIfEnumTypeIsDefined(string& enumTypeName);

void AddEnumSymbolIfNotExists(
        const string& symbol_name);

void UpdateEnumSymbolValues(vector<string>& values, string enum_id);

void CheckNoDuplicatesEnumList(vector<string>& enumValues, string& newValue);

void CheckNoDuplicatesFormalList(vector<tuple<string, string, bool>>& args, string& newArgId);

void AddVariableSymbolIfNotExists(string& symbol_name,
                                  string& type,
                                  bool is_enum_type);

void CheckIfAssignmentAllowed(string& lType, string& expType);

void CheckIfAssignmentAllowedEnum(string& enumType, string& expEnumType, string& varId);

string GetExpressionTypeById(string& id);

void HandleAssignmentForExistingVar(string& id, string& expType);

void CheckReturnValid(string& givenType);

void CheckFuncRetValNotVoid();

void CheckTypesMatch(vector<string>& expected, string& given);

void EnterLoop();

void LeaveLoop();

void CheckIfBreakInLoop();

void CheckIfContinueInLoop();


bool AreArgsEqual(vector<string> expListTypes, vector<tuple<string, string, bool>> fromRecord);

string CheckFunction(string& id, vector<string> expListTypes);

void CheckNumValidity(int byteNum);

string DetermineBinopReturnType(string& first, string& second);

void isExplicitCastAllowed(string& castToType, string& castFromType);


string FreshVar();

void DeclarePrintfAndExit();

void DeclarePrinti();

void DeclarePrint();

string CreateInitialIntegerVar(string value);

string CreateInitialByteVar(string value);

string ConvertIfByte(string type, string arg);

string DoArithmeticAction(string arg1, string arg2, char op, string retType);

void CompareAction(string arg1, string arg2, string op, string retType, vector<pair<int, BranchLabelIndex>>* &trueList, vector<pair<int, BranchLabelIndex>>* &falseList);

vector<pair<int, BranchLabelIndex>> CreatePatchList();

string GenLabel();

void HandleOr(vector<pair<int, BranchLabelIndex>>* &resTrueList, vector<pair<int, BranchLabelIndex>>* &resFalseList, vector<pair<int, BranchLabelIndex>>* B1TrueList, vector<pair<int, BranchLabelIndex>>* B1FalseList,vector<pair<int, BranchLabelIndex>>* B2TrueList, vector<pair<int, BranchLabelIndex>>* B2FalseList, string beforeSecondLabel);

void HandleAnd(vector<pair<int, BranchLabelIndex>>* &resTrueList, vector<pair<int, BranchLabelIndex>>* &resFalseList, vector<pair<int, BranchLabelIndex>>* B1TrueList, vector<pair<int, BranchLabelIndex>>* B1FalseList,vector<pair<int, BranchLabelIndex>>* B2TrueList, vector<pair<int, BranchLabelIndex>>* B2FalseList, string beforeSecondLabel);

string WriteStringToBuffer(string str);



#endif //HW3_PARSERFUNCTIONNEW_HPP