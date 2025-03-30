#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct FunctionSymbol {
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters; // name, type
    std::string returnType;
};

class SymbolTable {
public:
    bool addFunction(const FunctionSymbol& func);
    bool hasFunction(const std::string& name);
    FunctionSymbol* getFunction(const std::string& name);
    
private:
    std::unordered_map<std::string, FunctionSymbol> functions;
};