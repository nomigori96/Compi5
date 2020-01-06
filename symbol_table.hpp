#ifndef HW3_SYMBOL_TABLE_HPP
#define HW3_SYMBOL_TABLE_HPP


#include <stack>
#include <vector>
#include <string>
#include <tuple>

using namespace std;

enum Options {ENUM_VALUE, SYMBOL, DOESNT_EXIST};

class SymbolTableRecord {
protected:
    string name;
    int offset;
    string type;
    bool is_enum_type;

public:
    SymbolTableRecord(
            const string& name_p,
            int offset_p,
            const string& type_p,
            bool is_enum_type_p) :  name(name_p),
                                    offset(offset_p),
                                    type(type_p),
                                    is_enum_type(is_enum_type_p){}
    virtual ~SymbolTableRecord(){};
    SymbolTableRecord(const SymbolTableRecord &to_copy) = default;
    SymbolTableRecord& operator=(const SymbolTableRecord &to_copy) = default;
    string GetName() const {return name;}
    string GetType() const {return type;}
    int GetOffset() const {return offset;}
    bool IsEnumType() const {return is_enum_type;}
};


class FunctionSymbolTableRecord : public SymbolTableRecord {
private:
    vector<tuple<string, string, bool>> func_arguments;
    string func_ret_type;

public:
    FunctionSymbolTableRecord(
            const string& name_p,
            int offset_p,
            const vector<tuple<string,string, bool>>& func_arguments_p,
            const string& func_ret_value_type_p) : SymbolTableRecord(name_p, offset_p, "function", false),                                                          func_arguments(func_arguments_p),
                                                   func_ret_type(func_ret_value_type_p){}
    ~FunctionSymbolTableRecord() override = default;
    FunctionSymbolTableRecord(const FunctionSymbolTableRecord &to_copy) : SymbolTableRecord(to_copy),
                                                                          func_arguments(to_copy.func_arguments),
                                                                          func_ret_type(to_copy.func_ret_type){};
    FunctionSymbolTableRecord& operator=(const FunctionSymbolTableRecord &to_copy);
    vector<tuple<string,string, bool>> GetFuncArgs() const {return func_arguments;}
    void SetFuncArgs(vector<tuple<string,string, bool>>& funcArgs) {
        func_arguments = funcArgs;
    }
    string GetFuncReturnType() const {return func_ret_type;}

};

class EnumSymbolTableRecord : public SymbolTableRecord {
private:
    vector<string> enum_values;

public:
    EnumSymbolTableRecord(
            const string& name_p,
            int offset_p,
            const vector<string>& enum_values_p) : SymbolTableRecord(name_p, offset_p, "enum", false),
                                                   enum_values(enum_values_p){}
    ~EnumSymbolTableRecord() override = default;
    EnumSymbolTableRecord(const EnumSymbolTableRecord &to_copy) : SymbolTableRecord(to_copy),
                                                                  enum_values(to_copy.enum_values){};
    EnumSymbolTableRecord& operator=(const EnumSymbolTableRecord &to_copy);
    vector<string> GetEnumValues() const {return enum_values;}
    void SetEnumValues(vector<string>& enumValues) {
        enum_values = enumValues;
    }
    bool DoesValueExists(const string& value);

};

class SymbolTable{
private:
    stack<vector<SymbolTableRecord*>>* symbol_table;
    stack<int>* offsets_stack;

public:
    void InsertSymbol(
            const string& symbol_name,
            const string& type,
            bool is_enum_type);
    void InsertFunction(
            const string& symbol_name,
            const vector<tuple<string,string, bool>>& func_arguments,
            const string& func_return_value_type);
    void InsertEnum(
            const string& symbol_name,
            const vector<string>& enum_values);
    void InsertFunctionArgSymbol(
            const string& symbol_name,
            const string& type,
            int offset,
            bool is_enum_type);
    void OpenScope();
    void CloseCurrentScope();
    SymbolTableRecord* GetSymbolRecordById(const string& id);
    Options DoesSymbolExists(const string& id);
    SymbolTable();
    ~SymbolTable();
    string FindEnumTypeByGivenValue(const string& value);
    vector<SymbolTableRecord*> GetCurrentScope(){return symbol_table->top();};
};

#endif //HW3_SYMBOL_TABLE_HPP
