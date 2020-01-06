#include "symbol_table.hpp"

FunctionSymbolTableRecord& FunctionSymbolTableRecord::operator=(const FunctionSymbolTableRecord &to_copy){
    func_arguments = to_copy.func_arguments;
    func_ret_type = to_copy.func_ret_type;
    type = to_copy.type;
    name = to_copy.name;
    offset = to_copy.offset;
    is_enum_type = to_copy.is_enum_type;
    return *this;
}

EnumSymbolTableRecord& EnumSymbolTableRecord::operator=(const EnumSymbolTableRecord &to_copy){
    enum_values = to_copy.enum_values;
    type = to_copy.type;
    name = to_copy.name;
    offset = to_copy.offset;
    is_enum_type = to_copy.is_enum_type;
    return *this;
}

bool EnumSymbolTableRecord::DoesValueExists(const string& value){
    for (auto &enum_value : enum_values){
        if (enum_value == value){
            return true;
        }
    }
    return false;
}

void SymbolTable::InsertSymbol(
        const string& symbol_name,
        const string& type,
        bool is_enum_type)
{
    symbol_table->top().push_back(
            new SymbolTableRecord(
                symbol_name,
                offsets_stack->top(),
                type,
                is_enum_type));
    offsets_stack->top()++;
}

void SymbolTable::InsertFunction(
        const string& symbol_name,
        const vector<tuple<string,string, bool>>& func_arguments,
        const string& func_return_value_type)
{
    symbol_table->top().push_back(
            new FunctionSymbolTableRecord(
                    symbol_name,
                    0,
                    func_arguments,
                    func_return_value_type));
}

void SymbolTable::InsertEnum(
        const string& symbol_name,
        const vector<string>& enum_values)
{
    symbol_table->top().push_back(
            new EnumSymbolTableRecord(symbol_name, 0, enum_values));
}

void SymbolTable::OpenScope()
{
    vector<SymbolTableRecord*> new_scope;
    symbol_table->push(new_scope);
    int curr_offset = offsets_stack->top();
    offsets_stack->push(curr_offset);
}

void SymbolTable::CloseCurrentScope()
{
    vector<SymbolTableRecord*> curr_scope;
    curr_scope = symbol_table->top();
    for (auto &symbol : curr_scope) {
        delete symbol;
    }
    symbol_table->pop();
    offsets_stack->pop();
}

// Call this method only if you have checked earlier that DoesSymbolExists returns SYMBOL
SymbolTableRecord* SymbolTable::GetSymbolRecordById(const string& id)
{
    if (symbol_table->empty()){
        //shouldn't get here
        throw exception();
    }
    vector<SymbolTableRecord*> scope;
    scope = symbol_table->top();
    for (auto &symbol : scope) {
        if (symbol->GetName() == id){
            return symbol;
        }
    }
    symbol_table->pop();
    SymbolTableRecord* result = GetSymbolRecordById(id);
    symbol_table->push(scope);
    return result;
}

Options SymbolTable::DoesSymbolExists(const string& id)
{
    if (symbol_table->empty()){
        return DOESNT_EXIST;
    }
    vector<SymbolTableRecord*> scope;
    scope = symbol_table->top();
    for (auto &symbol : scope) {
        if (symbol->GetName() == id){
            return SYMBOL;
        }
        if ((symbol->GetType() == "enum" &&
                dynamic_cast<EnumSymbolTableRecord*>(symbol)->DoesValueExists(id) )){
            return ENUM_VALUE;
        }
    }
    symbol_table->pop();
    Options result = DoesSymbolExists(id);
    symbol_table->push(scope);
    return result;
}

SymbolTable::SymbolTable()
{
    symbol_table = new stack<vector<SymbolTableRecord*>>();
    offsets_stack = new stack<int>();
    offsets_stack->push(0);
    vector<tuple<string,string, bool>> print_arguments;
    print_arguments.push_back(tuple<string, string, bool>("STRING", "", false));
    SymbolTableRecord* print_record = new FunctionSymbolTableRecord("print", 0, print_arguments, "VOID");
    vector<tuple<string,string, bool>> printi_arguments;
    printi_arguments.push_back(tuple<string, string, bool>("INT", "", false));
    SymbolTableRecord* printi_record = new FunctionSymbolTableRecord("printi", 0, printi_arguments, "VOID");
    vector<SymbolTableRecord*> scope = vector<SymbolTableRecord*>();
    scope.push_back(print_record);
    scope.push_back(printi_record);
    symbol_table->push(scope);
}

SymbolTable::~SymbolTable()
{
    vector<SymbolTableRecord*> curr_scope;
    while (!symbol_table->empty()){
        curr_scope = symbol_table->top();
        for (auto &symbol : curr_scope){
            delete symbol;
        }
        symbol_table->pop();
    }

    delete symbol_table;
    delete offsets_stack;
}

void SymbolTable::InsertFunctionArgSymbol(
        const string& symbol_name,
        const string& type,
        int offset,
        bool is_enum_type)
{
    symbol_table->top().push_back(new SymbolTableRecord(
            symbol_name,
            offset,
            type,
            is_enum_type));
}

// Call this method only if you have checked earlier that DoesSymbolExists returns ENUM_VALUE
string SymbolTable::FindEnumTypeByGivenValue(const string& value){
    if (symbol_table->empty()){
        //shouldn't get here
        throw exception();
    }
    vector<SymbolTableRecord*> scope;
    scope = symbol_table->top();
    for (SymbolTableRecord* &symbol : scope) {
        if (symbol->GetType() == "enum"){
            vector<string> enumValues = dynamic_cast<EnumSymbolTableRecord*>(symbol)->GetEnumValues();
            for (auto &enumVal : enumValues){
                if (enumVal == value){
                    return symbol->GetName();
                }
            }
        }
    }
    symbol_table->pop();
    string result = FindEnumTypeByGivenValue(value);
    symbol_table->push(scope);
    return result;
}

