#include <SymbolTable.hpp>

bool SymbolTable::addFunction(const FunctionSymbol& func) {
    if (functions.count(func.name)) return false;
    functions[func.name] = func;
    return true;
}

bool SymbolTable::hasFunction(const std::string& name) {
    return functions.count(name);
}

FunctionSymbol* SymbolTable::getFunction(const std::string& name) {
    return hasFunction(name) ? &functions[name] : nullptr;
}