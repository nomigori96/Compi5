

#include <algorithm>
#include "ParserFunctions.hpp"


using namespace output;
using namespace std;

SymbolTable symbol_table;
string curr_function_return_type = "";

stack<bool> while_stack = stack<bool>();

extern int yylineno;

void CheckMainExists()
{
    if (symbol_table.DoesSymbolExists("main") == SYMBOL){
        SymbolTableRecord* mainRecord = symbol_table.GetSymbolRecordById("main");
        if (mainRecord->GetType() == "function"){
            FunctionSymbolTableRecord* mainFunctionRecord =
                    dynamic_cast<FunctionSymbolTableRecord*>(mainRecord);
            vector<tuple<string,string, bool>> main_record_args = mainFunctionRecord->GetFuncArgs();
            string ret = mainFunctionRecord->GetFuncReturnType();
            if (main_record_args.empty() && ret == "VOID"){
                return;
            }
        }
    }
    errorMainMissing();
    exit(0);
}

void OpenNewScope()
{
    symbol_table.OpenScope();
}

void CloseCurrentScope()
{
    endScope();
    vector<SymbolTableRecord*> currentScope = symbol_table.GetCurrentScope();
    for (auto &symbol : currentScope){
        if (symbol->GetType() == "function"){
            string retType = dynamic_cast<FunctionSymbolTableRecord*>(symbol)->GetFuncReturnType();
            vector<string> argTypes = MapArgsToTypes(dynamic_cast<FunctionSymbolTableRecord*>(symbol)->GetFuncArgs());
            string type = makeFunctionType(retType, argTypes);
            printID(symbol->GetName(), 0, type);
        } else if (symbol->GetType() == "enum"){
            //Do nothing
        }
        else {
            printID(symbol->GetName(), symbol->GetOffset(), symbol->GetType());
        }
    }
    for (auto &symbol : currentScope){
        if (symbol->GetType() == "enum"){
            vector<string> enumValues = dynamic_cast<EnumSymbolTableRecord*>(symbol)->GetEnumValues();
            printEnumType(symbol->GetName(), enumValues);
        }
    }
    symbol_table.CloseCurrentScope();
}

vector<string> MapArgsToTypes(vector<tuple<string, string, bool>> fromRecord){
    vector<string> onlyTypes;
    for(auto &currArg : fromRecord){
        string argType;
        tie(argType, ignore, ignore) = currArg;
        onlyTypes.push_back(argType);
    }
    return onlyTypes;
}


void AddFunctionSymbolIfNotExists(
        const string& symbol_name,
        const string& ret_type)
{
    if (symbol_table.DoesSymbolExists(symbol_name) != DOESNT_EXIST){
        errorDef(yylineno, symbol_name);
        exit(0);
    } else {
        vector<tuple<string,string,bool>> empty_args;
        symbol_table.InsertFunction(symbol_name, empty_args , ret_type);
    }
}

void UpdateCurrFunctionRetType(string& retType){
    curr_function_return_type = retType;
}

void AddFuncArgsToSymbolTable(vector<tuple<string,string,bool>>& args)
{
    int counter = -1;
    for (auto &arg : args) {
        symbol_table.InsertFunctionArgSymbol(
                get<1>(arg),
                get<0>(arg),
                counter,
                get<2>(arg));
        counter--;
    }
}

void UpdateFunctionSymbolArgs(vector<tuple<string,string,bool>>& args, string func_id){
    SymbolTableRecord* symbol_record = symbol_table.GetSymbolRecordById(func_id);
    FunctionSymbolTableRecord* func_record = dynamic_cast<FunctionSymbolTableRecord*>(symbol_record);
    func_record->SetFuncArgs(args);
}

void CheckIfIdIsShadowing(string& id){
    if (symbol_table.DoesSymbolExists(id) != DOESNT_EXIST){
        errorDef(yylineno, id);
        exit(0);
    }
}

void CheckIfEnumTypeIsDefined(string& enumTypeName){
    if(symbol_table.DoesSymbolExists(enumTypeName) == SYMBOL &&
            symbol_table.GetSymbolRecordById(enumTypeName)->GetType() == "enum"){
        return;
    }
    errorUndefEnum(yylineno, enumTypeName);
    exit(0);
}

void AddEnumSymbolIfNotExists(
        const string& symbol_name)
{
    if (symbol_table.DoesSymbolExists(symbol_name) != DOESNT_EXIST){
        errorDef(yylineno, symbol_name);
        exit(0);
    }
    else {
        vector<string> empty_values;
        symbol_table.InsertEnum(symbol_name, empty_values);
    }
}

void UpdateEnumSymbolValues(vector<string>& values, string enum_id){
    SymbolTableRecord* symbol_record = symbol_table.GetSymbolRecordById(enum_id);
    EnumSymbolTableRecord* enum_record = dynamic_cast<EnumSymbolTableRecord*>(symbol_record);
    enum_record->SetEnumValues(values);
}

void CheckNoDuplicatesEnumList(vector<string>& enumValues, string& newValue){
    if (find(enumValues.begin(), enumValues.end(), newValue) == enumValues.end()){
        return;
    }
    errorDef(yylineno, newValue);
    exit(0);
}

void CheckNoDuplicatesFormalList(vector<tuple<string, string, bool>>& args, string& newArgId){
    vector<string> onlyIds;
    for(auto &currArg : args){
        string argId;
        tie(ignore, argId, ignore) = currArg;
        onlyIds.push_back(argId);
    }
    if (find(onlyIds.begin(), onlyIds.end(), newArgId) == onlyIds.end()){
        return;
    }
    errorDef(yylineno, newArgId);
    exit(0);
}

void AddVariableSymbolIfNotExists(string& symbol_name,
                                  string& type,
                                  bool is_enum_type){
    if (symbol_table.DoesSymbolExists(symbol_name) != DOESNT_EXIST){
        errorDef(yylineno, symbol_name);
        exit(0);
    }
    symbol_table.InsertSymbol(symbol_name, type, is_enum_type);
}

void CheckIfAssignmentAllowed(string& lType, string& expType){
    if ((lType == "INT" && expType == "BYTE") || lType == expType){
        return;
    }
    errorMismatch(yylineno);
    exit(0);
}

void CheckIfAssignmentAllowedEnum(string& enumType, string& expEnumType, string& varId){
    if (expEnumType == enumType){
        return;
    }
    errorUndefEnumValue(yylineno, varId);
    exit(0);
}

string GetExpressionTypeById(string& id){
    if (symbol_table.DoesSymbolExists(id) == SYMBOL){
        SymbolTableRecord* wantedRecord = symbol_table.GetSymbolRecordById(id);
        string recordType = wantedRecord->GetType();
        if (recordType == "function" || recordType == "enum"){
            errorUndef(yylineno, id);
            exit(0);
        }
        return recordType;
    }
    if (symbol_table.DoesSymbolExists(id) == ENUM_VALUE){
        return string("enum ") + symbol_table.FindEnumTypeByGivenValue(id);
    }

    errorUndef(yylineno, id);
    exit(0);
}

void HandleAssignmentForExistingVar(string& id, string& expType){
    if (symbol_table.DoesSymbolExists(id) != SYMBOL){
        errorUndef(yylineno, id);
        exit(0);
    }
    SymbolTableRecord* wanted_record = symbol_table.GetSymbolRecordById(id);
    string wanted_type = wanted_record->GetType();
    if (wanted_type == "function" || wanted_type == "enum"){
        errorUndef(yylineno, id);
        exit(0);
    }
    if (wanted_record->IsEnumType()){
        CheckIfAssignmentAllowedEnum(wanted_type, expType, id);
    }
    else {
        CheckIfAssignmentAllowed(wanted_type, expType);
    }
}

void CheckReturnValid(string& givenType){
    CheckIfAssignmentAllowed(curr_function_return_type, givenType);
}

void CheckFuncRetValNotVoid(){
    if (curr_function_return_type == "VOID"){
        errorMismatch(yylineno);
        exit(0);
    }
}

void CheckTypesMatch(vector<string>& expected, string& given){
    for (string &expected_option : expected){
        if (given == expected_option){
            return;
        }
    }
    errorMismatch(yylineno);
    exit(0);
}

void EnterLoop(){
    while_stack.push(true);
}

void LeaveLoop(){
    while_stack.pop();
}

void CheckIfBreakInLoop(){
    if (while_stack.empty()){
        errorUnexpectedBreak(yylineno);
        exit(0);
    }
}

void CheckIfContinueInLoop(){
    if (while_stack.empty()){
        errorUnexpectedContinue(yylineno);
        exit(0);
    }
}


bool AreArgsEqual(vector<string> expListTypes, vector<tuple<string, string, bool>> fromRecord){
    if (expListTypes.size() != fromRecord.size()){
        return false;
    }
    auto expListTypeIterator = expListTypes.begin();
    auto fromRecordIterator = fromRecord.begin();
    while (expListTypeIterator != expListTypes.end()){
        if(*expListTypeIterator != get<0>(*fromRecordIterator) &&
                !(*expListTypeIterator == "BYTE" && get<0>(*fromRecordIterator) == "INT")){
            return false;
        }
        ++expListTypeIterator;
        ++fromRecordIterator;
    }
    return true;
}

string CheckFunction(string& id, vector<string> expListTypes){
    if (symbol_table.DoesSymbolExists(id) == SYMBOL){
        SymbolTableRecord* wantedRecord = symbol_table.GetSymbolRecordById(id);
        if (wantedRecord->GetType() != "function"){
            errorUndefFunc(yylineno, id);
            exit(0);
        }
        vector<tuple<string, string, bool>> fromRecord = dynamic_cast<FunctionSymbolTableRecord*>(wantedRecord)->GetFuncArgs();
        if (!AreArgsEqual(expListTypes, fromRecord)){
            vector<string> expectedTypes = MapArgsToTypes(fromRecord);
            errorPrototypeMismatch(yylineno, id, expectedTypes);
            exit(0);
        }
        return dynamic_cast<FunctionSymbolTableRecord*>(wantedRecord)->GetFuncReturnType();
    }
    errorUndefFunc(yylineno, id);
    exit(0);
}

void CheckNumValidity(int byteNum){
    if (byteNum > 255){
        errorByteTooLarge(yylineno, to_string(byteNum));
        exit(0);
    }
}

string DetermineBinopReturnType(string& first, string& second){
    if(first == "BYTE" && second == "BYTE"){
        return "BYTE";
    }
    return "INT";
}

void isExplicitCastAllowed(string& castToType, string& castFromType){
    if (!(castToType == "INT" && castFromType.find("enum") == 0)){
        errorMismatch(yylineno);
        exit(0);
    }
}


void DeclarePrintfAndExit(){
    string printfDecl = "declare i32 @printf(i8*, ...)";
    string exitDecl = "declare void @exit(i32)";
    string globalDecl1 = "@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"";
    string globalDecl2 = "@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"";
    CodeBuffer::instance().emit(printfDecl);
    CodeBuffer::instance().emit(exitDecl);
    CodeBuffer::instance().emit(globalDecl1);
    CodeBuffer::instance().emit(globalDecl2);
}

void DeclarePrinti(){
    string printi = "define void @printi(i32) {call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)ret void}";
    CodeBuffer::instance().emit(printi);
}

void DeclarePrint(){
    string print = "define void @print(i8*) {call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)ret void";
    CodeBuffer::instance().emit(print);
}

string FreshVar(){
    return "%var" + __COUNTER__;
}


string CreateInitialIntegerVar(string value){
    string action;
    string varName = FreshVar();
    action = varName + "= add i32 0, " + value;
    CodeBuffer::instance().emit(action);
    return varName;
}

string CreateInitialByteVar(string value){
    string action;
    string varName = FreshVar();
    action = varName + "= add i8 0, " + value;
    CodeBuffer::instance().emit(action);
    return varName;
}


string ConvertIfByte(string type, string arg){
    if(type == "BYTE"){
        string varName = FreshVar();
        string action = varName + " = zext i8 " + arg + " to i32";
        CodeBuffer::instance().emit(action);
        return varName;
    }
    return arg;
}

string WriteStringToBuffer(string str){
    int stringLength = str.length();
    string stringArr = FreshVar();
    string alloc = stringArr + " = alloca [" + to_string(stringLength + 1) + " * i8]";
    CodeBuffer::instance().emit(alloc);
    for (int i = 0; i < stringLength; i++){
        string currVar = FreshVar();
        string currentChar = currVar + " = getelementptr [" + to_string(stringLength + 1) + " * i8]," + "[" + to_string(stringLength + 1) + " * i8]* %stringArr, i8 0, i8 " + to_string(i);
        string store = "store i8 " + string(1, str[i]) + ", i8* " + currVar;
        CodeBuffer::instance().emit(currentChar);
        CodeBuffer::instance().emit(store);
    }
    string lastVar = FreshVar();
    string lastChar = lastVar + " = getelementptr [" + to_string(stringLength + 1) + " * i8]," + "[" + to_string(stringLength + 1) + " * i8]* %stringArr, i8 0, i8 " + to_string(stringLength);
    string storeLastVar = "store i8 " + string(1, '\0') + ", i8* " + lastVar;
    CodeBuffer::instance().emit(lastChar);
    CodeBuffer::instance().emit(storeLastVar);
    return stringArr;
}


string DoArithmeticAction(string arg1, string arg2, char op, string retType){
    string action;
    string varName = FreshVar();
    action = varName + " = ";
    string actionType;
    if(retType == "BYTE"){
        actionType = "i8";
    }
    else {
        actionType = "i32";
    }

    switch(op){
        case '+':
            action += "add " + actionType + " " + arg1 + " , " + arg2;
            break;
        case '-':
            action += "sub " + actionType + " " + arg1 + " , " + arg2;
            break;
        case '*':
            action += "mul " + actionType + " " + arg1 + " , " + arg2;
            break;
        case '/':
            string divCheck;
            string condVar = FreshVar();
            divCheck = condVar + " = icmp eq " + actionType + " 0, " + arg2;
            CodeBuffer::instance().emit(divCheck);
            divCheck = "br i1 " + condVar + ", label @, label @";
            int condBrToPatch = CodeBuffer::instance().emit(divCheck);
            string isZero = CodeBuffer::instance().genLabel();
            CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(std::pair(condBrToPatch, FIRST)), isZero);
            string arrVar = WriteStringToBuffer("Error division by zero");
            divCheck = "call void @print(" + arrVar + ")";
            CodeBuffer::instance().emit(divCheck);
            divCheck = "call void @exit(0)";
            CodeBuffer::instance().emit(divCheck);
            string isNotZero = CodeBuffer::instance().genLabel();
            CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(std::pair(condBrToPatch, SECOND)), isNotZero);
            action += "sdiv " + actionType + " " + arg1 + " , " + arg2;
            break;
        default:
            break;
    }
    CodeBuffer::instance().emit(action);
    return varName;
}


void CompareAction(string arg1, string arg2, string op, string retType, vector<pair<int, BranchLabelIndex>>* &trueList, vector<pair<int, BranchLabelIndex>>* &falseList){
    string action;
    string varName = FreshVar();
    action = varName + " = icmp ";
    string actionType;
    if(retType == "BYTE"){
        actionType = "i8";
    }
    else {
        actionType = "i32";
    }
    if (op == "=="){
        action += "eq " + actionType + " " + arg1 + " , " + arg2;
    }
    else if (op == "!="){
        action += "ne " + actionType + " " + arg1 + " , " + arg2;
    }
    else if (op == "<="){
        string cond = (actionType == "i8") ? "ule" : "sle";
        action += cond + " " + actionType + " " + arg1 + " , " + arg2;
    }
    else if (op == ">="){
        string cond = (actionType == "i8") ? "uge" : "sge";
        action += cond + " " + actionType + " " + arg1 + " , " + arg2;
    }
    else if (op == "<"){
        string cond = (actionType == "i8") ? "ult" : "slt";
        action += cond + " " + actionType + " " + arg1 + " , " + arg2;
    }
    else if (op == ">"){
        string cond = (actionType == "i8") ? "ugt" : "sgt";
        action += cond + " " + actionType + " " + arg1 + " , " + arg2;
    }
    CodeBuffer::instance().emit(action);
    string condBr = "br i1 " + varName + ", label @, label @";
    int patchLocation = CodeBuffer::instance().emit(condBr);
    trueList = new vector<pair<int, BranchLabelIndex>>(CodeBuffer::instance().makelist(pair<int, BranchLabelIndex >(patchLocation, FIRST)));
    falseList = new vector<pair<int, BranchLabelIndex>>(CodeBuffer::instance().makelist(pair<int, BranchLabelIndex >(patchLocation, SECOND)));
}

vector<pair<int, BranchLabelIndex>> CreatePatchList(){
    string action = "br label @";
    int location = CodeBuffer::instance().emit(action);
    return CodeBuffer::instance().makelist(pair<int, BranchLabelIndex>(location, FIRST));
}

string GenLabel(){
    return CodeBuffer::instance().genLabel();
}

void HandleAnd(vector<pair<int, BranchLabelIndex>>* &resTrueList, vector<pair<int, BranchLabelIndex>>* &resFalseList, vector<pair<int, BranchLabelIndex>>* B1TrueList, vector<pair<int, BranchLabelIndex>>* B1FalseList,vector<pair<int, BranchLabelIndex>>* B2TrueList, vector<pair<int, BranchLabelIndex>>* B2FalseList, string beforeSecondLabel){
    CodeBuffer::instance().bpatch(*B1TrueList, beforeSecondLabel);
    resTrueList = B2TrueList;
    resFalseList = new vector<pair<int, BranchLabelIndex>>(CodeBuffer::instance().merge(*B1FalseList, *B2FalseList));
}

void HandleOr(vector<pair<int, BranchLabelIndex>>* &resTrueList, vector<pair<int, BranchLabelIndex>>* &resFalseList, vector<pair<int, BranchLabelIndex>>* B1TrueList, vector<pair<int, BranchLabelIndex>>* B1FalseList,vector<pair<int, BranchLabelIndex>>* B2TrueList, vector<pair<int, BranchLabelIndex>>* B2FalseList, string beforeSecondLabel){
    CodeBuffer::instance().bpatch(*B1FalseList, beforeSecondLabel);
    resTrueList = new vector<pair<int, BranchLabelIndex>>(CodeBuffer::instance().merge(*B1TrueList, *B2TrueList));
    resFalseList = B2FalseList;
}

