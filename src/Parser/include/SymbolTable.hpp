#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct VariableSymbol {
  std::string name;
  std::string type;
  bool initialized;

  VariableSymbol() {}
  VariableSymbol(std::string name, std::string type, bool initialized = false)
      : name(name), type(type), initialized(initialized) {}
};

struct FunctionSymbol {
  std::string name;
  std::vector<std::pair<std::string, std::string>> parameters; // name, type
  std::string returnType;
  std::unordered_map<std::string, VariableSymbol> variables; // Track variables in function scope

  FunctionSymbol() {}
  FunctionSymbol(std::string name, std::vector<std::pair<std::string, std::string>> params, std::string returnType)
    : name(name), parameters(std::move(params)), returnType(std::move(returnType)) {}
};

class SymbolTable {
public:
  bool addFunction(const FunctionSymbol &func);
  bool hasFunction(const std::string &name);
  FunctionSymbol *getFunction(const std::string &name);

  bool addVariable(const std::string& funcName, VariableSymbol var);
  bool hasVariable(const std::string& funcName, const std::string& varName);
  std::optional<VariableSymbol*> getVariable(const std::string& funcName, const std::string& varName);

private:
  std::unordered_map<std::string, FunctionSymbol> functions;
};