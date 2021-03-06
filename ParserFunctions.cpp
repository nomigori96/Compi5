

#include <algorithm>
#include "ParserFunctions.hpp"


using namespace output;
using namespace std;

SymbolTable symbol_table;
string curr_function_return_type = "";

string local_vars_ptr = "";
string func_args_ptr = "";
string curr_func_num_args = "";

stack<bool> while_stack;
stack<string> while_exp_labels_stack;
stack<int> local_stack_ptr;

unsigned long long vars_counter = 0;
unsigned long long global_vars_counter = 0;

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
	if(local_stack_ptr.empty()){
		local_stack_ptr.push(0);
	}
	local_stack_ptr.push(local_stack_ptr.top());
}

void CloseCurrentScope()
{
    symbol_table.CloseCurrentScope();
	local_stack_ptr.pop();
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
    string printi = "define void @printi(i32) {";
    CodeBuffer::instance().emit(printi);
    printi = "call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)";
    CodeBuffer::instance().emit(printi);
    printi = "ret void";
    CodeBuffer::instance().emit(printi);
    printi = "}";
    CodeBuffer::instance().emit(printi);
}

void DeclarePrint(){
    string print = "define void @print(i8*) {";
    CodeBuffer::instance().emit(print);
    print = "call i32 (i8*, ...) @printf(i8* %0)";
    CodeBuffer::instance().emit(print);
    print = "ret void";
    CodeBuffer::instance().emit(print);
    print = "}";
    CodeBuffer::instance().emit(print);
}

string FreshVar(){
    return "%var" + to_string(vars_counter++);
}

string FreshGlobalVar(){
    return "@var" + to_string(global_vars_counter++);
}

string CreateInitialIntegerVar(string value){
    string action;
    string varName = FreshVar();
    action = varName + " = add i32 0, " + value;
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


string DoArithmeticAction(string arg1, string arg2, char op, string retType, string arg1Type, string arg2Type){
    string action;
    string varName = FreshVar();
	string finalVarName = FreshVar();
    string actionType = ConvertToLLVMType(retType);
    string divCheck;
    string condVar;
    int condBrToPatch;
    string isZero;
    string errorMsg;
    string stringStartPtr;
    string isNotZero;
	string convertedArg1;
	string convertedArg2;
    switch(op){
        case '+':
            action = varName + " = add " + actionType + " " + arg1 + " , " + arg2;
            break;
        case '-':
            action = varName + " = sub " + actionType + " " + arg1 + " , " + arg2;
            break;
        case '*':
            action = varName + " = mul " + actionType + " " + arg1 + " , " + arg2;
            break;
        case '/':
            condVar = FreshVar();
            divCheck = condVar + " = icmp eq " + actionType + " 0, " + arg2;
            CodeBuffer::instance().emit(divCheck);
            divCheck = "br i1 " + condVar + ", label @, label @";
            condBrToPatch= CodeBuffer::instance().emit(divCheck);
            isZero = GenLabel();
            CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(std::pair<int, BranchLabelIndex>(condBrToPatch, FIRST)), isZero);
            errorMsg = "Error division by zero";
            stringStartPtr = SaveStringAsGlobalVar(errorMsg);
            divCheck = "call void @print(i8* " + stringStartPtr + ")";
            CodeBuffer::instance().emit(divCheck);
            divCheck = "call void @exit(i32 0)";
            CodeBuffer::instance().emit(divCheck);
            isNotZero = GenLabel();
            CodeBuffer::instance().bpatch(CodeBuffer::instance().makelist(std::pair<int, BranchLabelIndex>(condBrToPatch, SECOND)), isNotZero);
			convertedArg1 = ConvertIfByte(arg2Type == "BYTE" ? arg1Type : "INT", arg1);
			convertedArg2 = ConvertIfByte(arg1Type == "BYTE" ? arg2Type : "INT", arg2);
            action = varName + " = sdiv i32 " + convertedArg1 + " , " + convertedArg2;
            break;
        default:
            break;
    }
    CodeBuffer::instance().emit(action);
	if (retType == "BYTE" && op == '/'){
		action = finalVarName + " = trunc i32 " + varName + " to i8";
		CodeBuffer::instance().emit(action);
	}
	else {
		finalVarName = varName;
	}
    return finalVarName;
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
    auto brToPatch = CreatePatchList();
    string addedLabel = CodeBuffer::instance().genLabel();
    CodeBuffer::instance().bpatch(brToPatch, addedLabel);
    return addedLabel;
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

string HandleExpId(string id){
    if (symbol_table.DoesSymbolExists(id) == SYMBOL){
        SymbolTableRecord* wantedRecord = symbol_table.GetSymbolRecordById(id);
        string recordType = wantedRecord->GetType();
        int offset = wantedRecord->GetOffset();
        string action;
        string loadFrom = FreshVar();
        if (offset < 0){
            action = loadFrom + " = getelementptr [" + curr_func_num_args + " x i32], [" + curr_func_num_args + " x i32]* " + func_args_ptr + ", i32 0, i32 " + to_string(abs(offset) - 1);
        }
        else {
            action = loadFrom + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_ptr + ", i32 0, i32 " + to_string(offset);
        }
        CodeBuffer::instance().emit(action);
        string existingVarReg = FreshVar();
        action = existingVarReg + " = load i32, i32* " + loadFrom;
        CodeBuffer::instance().emit(action);
        string llvmType = ConvertToLLVMType(recordType);
        string existingVarConvertedReg = existingVarReg;
        if(llvmType != "i32"){
            existingVarConvertedReg = FreshVar();
            action = existingVarConvertedReg + " = trunc i32 " + existingVarReg + " to " + llvmType;
            CodeBuffer::instance().emit(action);
        }

        return existingVarConvertedReg;
    }
    //id is enum value
    string enumType = symbol_table.FindEnumTypeByGivenValue(id);
    SymbolTableRecord* enumTypeRecord = symbol_table.GetSymbolRecordById(enumType);
    vector<string> enumValues = dynamic_cast<EnumSymbolTableRecord*>(enumTypeRecord)->GetEnumValues();
    vector<string>::iterator itr = find(enumValues.begin(), enumValues.end(), id);
    int enumValueNum = distance(enumValues.begin(), itr);
    string enumValueVar = FreshVar();
    string action = enumValueVar + " = add i32 0, " + to_string(enumValueNum);
    CodeBuffer::instance().emit(action);
    return  enumValueVar;
}

string ConvertToLLVMType(string type){
    if (type == "BOOL"){
        return "i1";
    }
    else if (type == "INT" || type.find("enum") == 0){
        return "i32";
    }
    else if (type == "VOID"){
        return "void";
    }
    else {
        return "i8";
    }
}

void DefineFunc(string funcName, string funcRetType, vector<tuple<string, string, bool>>* args){
    string llvmRetType = ConvertToLLVMType(funcRetType);
    string funcDecl = "define " + llvmRetType + " @" + funcName + "(";
    if (!args->empty()){
        func_args_ptr = FreshVar();
        curr_func_num_args = to_string(args->size());
        funcDecl += "[" + curr_func_num_args + " x i32]* " + func_args_ptr;
        funcDecl += ") {";
    }
    else {
        funcDecl += ") {";
    }
    CodeBuffer::instance().emit(funcDecl);
}

void AllocateLocalVars(){
    local_stack_ptr.push(0);
    local_vars_ptr = FreshVar();
    string action = local_vars_ptr + " = alloca [50 x i32]";
    CodeBuffer::instance().emit(action);
}

string CloseFuncDefinition(string funcRetType){
    string endFuncLabel = GenLabel();
    string action;
    if (funcRetType == "VOID"){
        action = "ret void";
        CodeBuffer::instance().emit(action);
    }
    else {
        string llvmRetType = ConvertToLLVMType(funcRetType);
        action = "ret " + llvmRetType + " 0";
        CodeBuffer::instance().emit(action);
    }
    action = "}";
    CodeBuffer::instance().emit(action);
    return endFuncLabel;
}

void CreateNewVarDefaultValue(){
    string storeTo = FreshVar();
    string action = storeTo + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_ptr + ", i32 0, i32 " + to_string(local_stack_ptr.top());
    CodeBuffer::instance().emit(action);
    action = "store i32 0, i32* " + storeTo;
    CodeBuffer::instance().emit(action);
    local_stack_ptr.top()++;
}

void CreateNewVarGivenValue(string type, string toStore, string toStoreType){
    string storeTo = FreshVar();
    string action = storeTo + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_ptr + ", i32 0, i32 " + to_string(local_stack_ptr.top());
    CodeBuffer::instance().emit(action);
    string updatedToStore = FreshVar();
    if (type == "INT" || type.find("enum") == 0){
        string convertedToStore = toStore;
        if (toStoreType == "BYTE"){
            convertedToStore = FreshVar();
            string convertAction = convertedToStore + " = zext i8 " + toStore + " to i32";
            CodeBuffer::instance().emit(convertAction);
        }
        action = updatedToStore + " = add i32 0, " + convertedToStore;
        CodeBuffer::instance().emit(action);
    }
    else if (type == "BOOL"){
        action = updatedToStore + " = zext i1 " + toStore + " to i32";
        CodeBuffer::instance().emit(action);
    }
    else {
        action = updatedToStore + " = zext i8 " + toStore + " to i32";
        CodeBuffer::instance().emit(action);
    }
    action = "store i32 " + updatedToStore + ", i32* " + storeTo;
    CodeBuffer::instance().emit(action);
    local_stack_ptr.top()++;
}

void UpdateVar(string type, string toStore, string varId, string toStoreType){
    SymbolTableRecord* varRecord = symbol_table.GetSymbolRecordById(varId);
    int varStackOffset = varRecord->GetOffset();
    string action;
    string storeTo = FreshVar();
    if (varStackOffset < 0){
        action = storeTo + " = getelementptr [" + curr_func_num_args + " x i32], [" + curr_func_num_args + " x i32]* " + func_args_ptr + ", i32 0, i32 " + to_string(abs(varStackOffset) - 1);
    }
    else {
        action = storeTo + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_ptr + ", i32 0, i32 " + to_string(varStackOffset);
    }
    CodeBuffer::instance().emit(action);
    string updatedToStore = FreshVar();
    if (type == "INT" || type.find("enum") == 0){
        string convertedToStore = toStore;
        if (toStoreType == "BYTE"){
            convertedToStore = FreshVar();
            string convertAction = convertedToStore + " = zext i8 " + toStore + " to i32";
            CodeBuffer::instance().emit(convertAction);
        }
        action = updatedToStore + " = add i32 0, " + convertedToStore;
        CodeBuffer::instance().emit(action);
    }
    else if (type == "BOOL"){
        action = updatedToStore + " = zext i1 " + toStore + " to i32";
        CodeBuffer::instance().emit(action);
    }
    else {
        action = updatedToStore + " = zext i8 " + toStore + " to i32";
        CodeBuffer::instance().emit(action);
    }
    action = "store i32 " + updatedToStore + ", i32* " + storeTo;
    CodeBuffer::instance().emit(action);
}

void BPatchList(string labelToPatch, vector<pair<int, BranchLabelIndex >>* someList){
    CodeBuffer::instance().bpatch(*someList, labelToPatch);
}

vector<pair<int, BranchLabelIndex >> MergePatchLists(vector<pair<int, BranchLabelIndex >> list1, vector<pair<int, BranchLabelIndex >> list2){
    return CodeBuffer::instance().merge(list1, list2);
}

void EnterLoopPushExpLabel(string expLabel){
    while_exp_labels_stack.push(expLabel);
}

void ExitLoopPopExpLabel(){
    while_exp_labels_stack.pop();
}

string GetWhileExpLabel(){
    return while_exp_labels_stack.top();
}

string AllocateFuncArgs(int numArgs, vector<string> args, vector<string> argsTypes){
    string funcArgsVar = FreshVar();
    string action = funcArgsVar + " = alloca [" + to_string(numArgs) + " x i32]";
    CodeBuffer::instance().emit(action);
    for (int i = 0; i < numArgs; i++){
        string currVar = FreshVar();
        string currentVarPtrAction = currVar + " = getelementptr [" + to_string(numArgs) + " x i32]," + "[" + to_string(numArgs) + " x i32]* " + funcArgsVar + ", i32 0, i32 " + to_string(i);
        string currType = argsTypes[i];
        string updatedToStore = FreshVar();
        if (currType == "INT" || currType.find("enum") == 0){
            action = updatedToStore + " = add i32 0, " + args[i];
            CodeBuffer::instance().emit(action);
        }
        else if (currType == "BOOL"){
            action = updatedToStore + " = zext i1 " + args[i] + " to i32";
            CodeBuffer::instance().emit(action);
        }
        else {
            action = updatedToStore + " = zext i8 " + args[i] + " to i32";
            CodeBuffer::instance().emit(action);
        }
        string store = "store i32 " + updatedToStore + ", i32* " + currVar;
        CodeBuffer::instance().emit(currentVarPtrAction);
        CodeBuffer::instance().emit(store);
    }
    return funcArgsVar;
}

void CallPrintFunction(string toPrintPtr){
    string action = "call void @print(i8* " + toPrintPtr + ")";
    CodeBuffer::instance().emit(action);
}

void CallPrintiFunction(string toPrintPtr, string argType){
    string convertedReg = toPrintPtr;
    if (argType == "BYTE"){
        convertedReg = FreshVar();
        string convertAction = convertedReg + " = zext i8 " + toPrintPtr + " to i32";
        CodeBuffer::instance().emit(convertAction);
    }
    string action = "call void @printi(i32 " + convertedReg + ")";
    CodeBuffer::instance().emit(action);
}

string CallFunction(string argsAllocationVar, string funcName, int numArgs){
    string returnedFuncVal = FreshVar();
    SymbolTableRecord* wantedRecord = symbol_table.GetSymbolRecordById(funcName);
    string retType = dynamic_cast<FunctionSymbolTableRecord*>(wantedRecord)->GetFuncReturnType();
    string llvmRetType = ConvertToLLVMType(retType);
    string action;
    if (llvmRetType == "void"){
        action = "call " + llvmRetType + " @" + funcName + "( [" + to_string(numArgs) + " x i32]* " + argsAllocationVar + ")";
        returnedFuncVal = "";
    }
    else {
        action = returnedFuncVal + " = call " + llvmRetType + " @" + funcName + "( [" + to_string(numArgs) + " x i32]* " + argsAllocationVar + ")";
    }

    CodeBuffer::instance().emit(action);

    return returnedFuncVal;
}

string CallFunctionNoArgs(string funcName){
    string returnedFuncVal = FreshVar();
    SymbolTableRecord* wantedRecord = symbol_table.GetSymbolRecordById(funcName);
    string retType = dynamic_cast<FunctionSymbolTableRecord*>(wantedRecord)->GetFuncReturnType();
    string llvmRetType = ConvertToLLVMType(retType);
    string action;
    if (llvmRetType == "void"){
        action = "call " + llvmRetType + " @" + funcName + "()";
        returnedFuncVal = "";
    }
    else {
        action = returnedFuncVal + " = call " + llvmRetType + " @" + funcName + "()";
    }
    CodeBuffer::instance().emit(action);
    return returnedFuncVal;
}

void emitReturn(string retType, string varToReturn){
    string action;
    if (curr_function_return_type == "VOID"){
        action = "ret void";
    }
    else {
		string convertedVarToReturn = varToReturn;
		if(curr_function_return_type == "INT"){
			convertedVarToReturn = ConvertIfByte(retType, varToReturn);
		}
        action = "ret " + ConvertToLLVMType(curr_function_return_type) + " " + convertedVarToReturn;
    }
    CodeBuffer::instance().emit(action);
}

void PrintLLVMCode(){
    CodeBuffer::instance().printCodeBuffer();
}

void HandleBoolVarAsExp(string regWithBoolValueName, vector<pair<int, BranchLabelIndex>>* &trueList, vector<pair<int, BranchLabelIndex>>* &falseList){
    string condBr = "br i1 " + regWithBoolValueName + ", label @, label @";
    int patchLocation = CodeBuffer::instance().emit(condBr);
    trueList = new vector<pair<int, BranchLabelIndex>>(CodeBuffer::instance().makelist(pair<int, BranchLabelIndex >(patchLocation, FIRST)));
    falseList = new vector<pair<int, BranchLabelIndex>>(CodeBuffer::instance().makelist(pair<int, BranchLabelIndex >(patchLocation, SECOND)));
}

string SaveBoolExpInReg(vector<pair<int, BranchLabelIndex>>* trueList, vector<pair<int, BranchLabelIndex>>* falseList){
    string regToSaveIn = FreshVar();
    string trueLabel = GenLabel();
    auto trueBrToPatch = CreatePatchList();
    string falseLabel = GenLabel();
    auto falseBrToPatch = CreatePatchList();
    string phiLabel = CodeBuffer::instance().genLabel();
    CodeBuffer::instance().bpatch(*trueList, trueLabel);
    CodeBuffer::instance().bpatch(*falseList, falseLabel);
    CodeBuffer::instance().bpatch(trueBrToPatch, phiLabel);
    CodeBuffer::instance().bpatch(falseBrToPatch, phiLabel);
    string phiAction = regToSaveIn + " = phi i1 [true, %" + trueLabel + "], [false, %" + falseLabel + "]";
    CodeBuffer::instance().emit(phiAction);
    return regToSaveIn;
}

string HandleStrSpecialChars(string strToSave){
	string updatedStr;
	for (auto strIter = strToSave.begin(); strIter != strToSave.end(); ++strIter){
		if(*strIter == '\\'){
			strIter++;
			if(*strIter == 'n'){
				updatedStr += '\x0A';
			} else if (*strIter == 'r'){
				updatedStr += '\x0D';
			} else if (*strIter == 't'){
				updatedStr += '\x09';
			} else if (*strIter == '\"'){
				updatedStr += '\x22';
			} else if (*strIter == '\\'){
				updatedStr += '\x5C';
			}
		}
		else {
			updatedStr += *strIter;
		}
	}
	return updatedStr;
}

string SaveStringAsGlobalVar(string strToSave){
	string globalVarName = FreshGlobalVar();
	//string convertedStr = HandleStrSpecialChars(strToSave);
	int strLen = strToSave.length();
	string declToEmit = globalVarName + " = constant [" + to_string(strLen + 2) + " x i8] c\""+ strToSave +"\\0A\\00\"";
	CodeBuffer::instance().emitGlobal(declToEmit);
	string strAsPtr = FreshVar();
	string getPtrToStrStartAction = strAsPtr + " = getelementptr [" + to_string(strLen + 2) + " x i8]," + "[" + to_string(strLen + 2) + " x i8]* " + globalVarName + ", i32 0, i32 0";
	CodeBuffer::instance().emit(getPtrToStrStartAction);
	return strAsPtr;
}

void PrintAllGlobals(){
	CodeBuffer::instance().printGlobalBuffer();
}
