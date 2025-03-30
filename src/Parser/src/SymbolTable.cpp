#include <SymbolTable.hpp>
#include <iostream>
#include <optional>
#include <string>

bool SymbolTable::addFunction(const FunctionSymbol &func) {
  if (functions.find(func.name) != functions.end()) {
    return false; // Function already exists
  }
  functions[func.name] = std::move(func);
  return true;
}

bool SymbolTable::hasFunction(const std::string &name) {
  return functions.find(name) != functions.end();
}

FunctionSymbol *SymbolTable::getFunction(const std::string &name) {
  return hasFunction(name) ? &functions[name] : nullptr;
}

bool SymbolTable::addVariable(const std::string& funcName, VariableSymbol var) {
    if (!hasFunction(funcName)) {
        std::cerr << "Error: Function '" << funcName << "' does not exist." << std::endl;
        return false;
    }

    auto& func = functions[funcName];
    if (func.variables.find(var.name) != func.variables.end()) {
        std::cerr << "Error: Variable '" << var.name << "' already declared in function '" << funcName << "'." << std::endl;
        return false;
    }

    func.variables[var.name] = std::move(var);
    return true;
}

bool SymbolTable::hasVariable(const std::string& funcName, const std::string& varName) {
    if (!hasFunction(funcName)) return false;
    return functions[funcName].variables.find(varName) != functions[funcName].variables.end();
}

std::optional<VariableSymbol*> SymbolTable::getVariable(const std::string& funcName, const std::string& varName) {
    if (!hasFunction(funcName)) return std::nullopt;
    auto& func = functions[funcName];

    if (func.variables.find(varName) != func.variables.end()) {
        return &func.variables[varName];
    }

    return std::nullopt;
}