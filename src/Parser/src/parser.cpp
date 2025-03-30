#include "AST.hpp"
#include "SymbolTable.hpp"
#include "token.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <parser.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

void Parser::enforceEntryPoint() {
  if (!symTable.hasFunction("chính")) {
    throw std::runtime_error(
        "Program must define a 'chính' function as the entry point.");
  }

  FunctionSymbol *mainFunc = symTable.getFunction("chính");

  // Ensure 'chính' has optional or no parameters
  if (!mainFunc->parameters.empty()) {
    throw std::runtime_error("'Chính' function should not take parameters.");
  }

  // Ensure 'chính' has return type 'rỗng'
  if (mainFunc->returnType != "rỗng") {
    throw std::runtime_error("'chính' function must return 'rỗng'.");
  }
}

void Parser::advance() { current = lexer.nextToken(); }

bool Parser::check(TokenType type) const { return current.type == type; }

bool Parser::match(TokenType type) {
  if (current.type == type) {
    advance();
    return true;
  }
  return false;
}

std::string Parser::getLineSnippet(int errorLine) {
  std::istringstream stream(lexer.getSource());
  std::string lineContent;
  for (int i = 1; i <= errorLine; i++) {
    std::getline(stream, lineContent);
  }
  return lineContent;
}

void Parser::reportError(const ParseError &e) {
  std::cerr << e.what() << std::endl;
  std::string lineContent = getLineSnippet(e.line);
  std::cerr << "  " << lineContent << std::endl;
  
  // Add safety check
  int spaces = std::max(0, e.column - 1);
  std::cerr << "  " << std::string(spaces, ' ') << "^" << std::endl;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (check(TokenType::Number)) {
    auto expr = std::make_unique<LiteralExpr>(current.value, TokenType::Number);
    advance();
    return expr;
  }

  if (check(TokenType::String)) {
      auto expr = std::make_unique<LiteralExpr>(current.value, TokenType::String);
      advance();
      return expr;
  }

  if (check(TokenType::Boolean)) {
      auto expr = std::make_unique<LiteralExpr>(current.value, TokenType::Boolean);
      advance();
      return expr;
  }

  if (check(TokenType::Identifier)) {
    std::string identifier = current.value;
    advance();

    auto expr = std::make_unique<VarExpr>(identifier);

    // If the next token is '(', parse function call
    return parseCall(std::move(expr));
  }

  if (match(TokenType::OpenParen)) {
    auto expr = parseExpression(); // Recursively parse inside parentheses
    if (!match(TokenType::CloseParen)) {
      throw ParseError(current.line, current.column,
                       "Expected ')' after expression.");
    }
    return expr;
  }

  throw ParseError(current.line, current.column,
                   "Unexpected token: " + current.value);
}

std::unique_ptr<Expr> Parser::parseUnary() {
  if (check(TokenType::Operator) &&
      (current.value == "-" || current.value == "!" || current.value == "~")) {
    std::string op = current.value;
    advance();
    auto right = parseUnary();
    return std::make_unique<UnaryExpr>(op, std::move(right));
  }
  return parsePrimary();
}

std::unique_ptr<Expr> Parser::parseBinary(int precedence,
                                          std::unique_ptr<Expr> left) {
  while (check(TokenType::Operator)) {
    std::string op = current.value;
    int opPrecedence = getPrecedence(op);

    if (opPrecedence < precedence)
      break;

    advance(); // Consume operator
    auto right = parseBinary(opPrecedence + 1, parseUnary());
    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }
  return left;
}

std::unique_ptr<Expr> Parser::parseExpression() {
  return parseBinary(0, parseUnary());
}

std::unique_ptr<Expr> Parser::parseCall(std::unique_ptr<Expr> callee) {
  if (!match(TokenType::OpenParen))
    return callee;

  std::vector<std::unique_ptr<Expr>> arguments;
  if (!check(TokenType::CloseParen)) { // Handle empty argument list
    do {
      arguments.push_back(parseExpression());
    } while (match(TokenType::Comma));

    if (!match(TokenType::CloseParen)) {
      throw ParseError(current.line, current.column,
                       "Expected ')' after function arguments.");
    }
  }

  return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Stmt> Parser::parseBlockStatement() {
  std::vector<std::unique_ptr<Stmt>> statements;

  while (!check(TokenType::CloseBrace) && !check(TokenType::EndOfFile)) {
    statements.push_back(parseStatement());
  }

  if (!match(TokenType::CloseBrace)) {
    throw ParseError(current.column, current.line,
                     "Expected '}' at the end of a block.");
  }

  return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
  if (!match(TokenType::OpenParen)) {
    throw ParseError(current.line, current.column, "Expected '(' after 'nếu'.");
  }

  auto condition = parseExpression();

  if (!match(TokenType::CloseParen)) {
    throw ParseError(current.line, current.column,
                     "Expected ')' after nếu condition.");
  }

  auto thenBranch = parseStatement();
  std::unique_ptr<Stmt> elseBranch = nullptr;

  if (match(TokenType::Else)) {
    elseBranch = parseStatement();
  }

  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
  if (!match(TokenType::OpenParen)) {
    throw ParseError(current.line, current.column,
                     "Expected '(' after 'trong khi'.");
  }

  auto condition = parseExpression();

  if (!match(TokenType::CloseParen)) {
    throw ParseError(current.line, current.column,
                     "Expected ')' after while condition.");
  }

  auto body = parseStatement();

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseForStatement() {
  if (!match(TokenType::OpenParen)) {
    throw ParseError(current.line, current.column, "Expected '(' after 'cho'.");
  }

  std::unique_ptr<Stmt> initializer;
  if (!match(TokenType::Semicolon)) {
    initializer = parseStatement();
  }

  auto condition = match(TokenType::Semicolon) ? nullptr : parseExpression();
  if (!match(TokenType::Semicolon)) {
    throw ParseError(current.line, current.column,
                     "Expected ';' after loop condition.");
  }

  std::unique_ptr<Expr> increment =
      match(TokenType::CloseParen) ? nullptr : parseExpression();
  if (!match(TokenType::CloseParen)) {
    throw ParseError(current.line, current.column,
                     "Expected ')' after for clause.");
  }

  auto body = parseStatement();

  return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                                   std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseVarDeclStatement() {
  if (!check(TokenType::Identifier)) {
    throw ParseError(current.line, current.column,
                     "Expected variable name after 'biến'.");
  }
  std::string name = current.value;
  advance();

  std::optional<std::string> typeAnnotation;
  if (match(TokenType::Colon)) { // Check for optional type annotation
    if (!check(TokenType::Identifier)) {
      throw ParseError(current.line, current.column,
                       "Expected type name after ':'.");
    }
    typeAnnotation = handleType();
    if (typeAnnotation == "rỗng")
      throw ParseError(current.line, current.column, "Variable type cannot be 'rỗng'.");
  }

  std::unique_ptr<Expr> initializer = nullptr;
  // Check if current token is an operator and specifically "="
  if (check(TokenType::Operator) && current.value == "=") {
    advance(); // Now advance past the "=" operator
    initializer = parseExpression();
  }

  // If initializer and type annotation isn't declared
  if (!initializer && !typeAnnotation) {
    throw ParseError(current.line, current.column,
                     "Uninitialized variable must have a type annotation.");
  }

  // Type inference: If type annotation is missing, infer from initializer
  if (!typeAnnotation && initializer) {
    try {
      typeAnnotation = inferTypeFromExpression(initializer.get());
    } catch (const std::runtime_error &e) {
      throw ParseError(current.line, current.column, e.what());
    }
  }

  // Ensure type compatibility if both annotation and initializer exist
  if (typeAnnotation && initializer) {
    std::string inferredType;
    try {
      inferredType = inferTypeFromExpression(initializer.get());
    } catch (const std::runtime_error &e) {
      throw ParseError(current.line, current.column, e.what());
    }

    if (*typeAnnotation != inferredType) {
      throw ParseError(current.line, current.column,
                       "Type mismatch: Expected '" + *typeAnnotation +
                           "', but got '" + inferredType + "'.");
    }
  }

  if (!match(TokenType::Semicolon)) {
    throw ParseError(current.line, current.column,
                     "Expected ';' after variable declaration.");
  }

  if (!symTable.addVariable(currentFunction ? currentFunction->name : "",
                            {name, typeAnnotation.value_or(""),
                             static_cast<bool>(initializer)})) {
    throw ParseError(current.line, current.column,
                     "Variable '" + name + "' already declared.");
  }

  return std::make_unique<VarDeclStmt>(name, std::move(typeAnnotation),
                                       std::move(initializer));
}

std::unique_ptr<Stmt> Parser::parseFunction() {
  if (!check(TokenType::Identifier)) {
    throw ParseError(current.line, current.column, "Expected function name.");
  }

  std::string name = current.value;
  advance();

  if (!match(TokenType::OpenParen)) {
    throw ParseError(current.line, current.column,
                     "Expected '(' after function name.");
  }

  std::vector<std::pair<std::string, std::string>> parameters;
  if (!match(TokenType::CloseParen)) { // Handle parameters
    do {
      if (!check(TokenType::Identifier)) {
        throw ParseError(current.line, current.column,
                         "Expected parameter name.");
      }
      std::string paramName = current.value;
      advance();

      std::string paramType;
      if (!match(TokenType::Colon)) {
        throw ParseError(current.line, current.column,
                         "Expected : after parameter name.");
      }

      if (!check(TokenType::Identifier)) {
        throw ParseError(current.line, current.column,
                         "Expected parameter type.");
      }
      paramType = handleType();
      if (paramType == "rỗng")
        throw ParseError(current.line, current.column, "Parameter type cannot be 'rỗng'.");

      parameters.emplace_back(paramName, paramType);
    } while (match(TokenType::Comma));

    if (!match(TokenType::CloseParen)) {
      throw ParseError(current.line, current.column,
                       "Expected ')' after parameters.");
    }
  }

  std::string returnType = "rỗng"; // Rename it to something else
  if (match(TokenType::Colon)) {
    if (!check(TokenType::Identifier)) {
      throw ParseError(current.line, current.column,
                       "Expected return type after ':'.");
    }
    returnType = handleType();
  }

  FunctionSymbol funcSymbol{name, std::move(parameters), returnType};
  if (!symTable.addFunction(funcSymbol)) {
    std::stringstream errorMsg;
    errorMsg << "Line " << current.line << ", Column " << current.column
             << ": Function '" << name << "' is already defined.";
    throw std::runtime_error(errorMsg.str());
  }

  // Set current function before passing its body
  std::optional<FunctionSymbol> previousFunction = currentFunction;
  currentFunction = funcSymbol;

  if (!match(TokenType::OpenBrace)) {
    throw ParseError(current.line, current.column,
                     "Expected '{' before function body.");
  }

  std::unique_ptr<Stmt> bodyStmt = parseBlockStatement();
  std::unique_ptr<BlockStmt> body(
      dynamic_cast<BlockStmt *>(bodyStmt.release()));

  if (!body) {
    throw ParseError(current.line, current.column,
                     "Expected a block statement for function body.");
  }

  currentFunction = previousFunction;

  return std::make_unique<FunctionStmt>(name, std::move(parameters), returnType,
                                        std::move(body));
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
  if (!currentFunction) {
    throw ParseError(current.line, current.column,
                     "Return statement outside of a function.");
  }

  std::unique_ptr<Expr> returnExpr = nullptr;
  if (!match(TokenType::Semicolon)) {
    returnExpr = parseExpression();
    if (!match(TokenType::Semicolon)) {
      throw ParseError(current.line, current.column,
                       "Expected ';' after return expression.");
    }
  }

  // Ensure return type compatibility
  if (returnExpr) {
    std::string inferredReturnType = inferTypeFromExpression(returnExpr.get());
    if (currentFunction->returnType == "rỗng") {
      throw ParseError(current.line, current.column,
                       "Cannot return a value from a 'rỗng' function.");
    }
    if (inferredReturnType != currentFunction->returnType) {
      throw ParseError(current.line, current.column,
                       "Return type mismatch: Expected '" +
                           currentFunction->returnType + "', got '" +
                           inferredReturnType + "'.");
    }
  }

  return std::make_unique<ReturnStmt>(std::move(returnExpr));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
  if (match(TokenType::If))
    return parseIfStatement();
  if (match(TokenType::While))
    return parseWhileStatement();
  if (match(TokenType::For))
    return parseForStatement();
  if (match(TokenType::OpenBrace))
    return parseBlockStatement();
  if (match(TokenType::Var))
    return parseVarDeclStatement();
  if (match(TokenType::Function))
    return parseFunction();
  if (match(TokenType::Return))
    return parseReturnStatement();

  auto expr = parseExpression();
  if (!match(TokenType::Semicolon)) {
    throw ParseError(current.line, current.column,
                     "Expected ';' after expression.");
  }
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::vector<std::unique_ptr<Stmt>> Parser::parseProgram() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (current.type != TokenType::EndOfFile) {
    statements.push_back(parseStatement());
  }
  enforceEntryPoint();
  return statements;
}

Parser::Parser(Lexer lexer) : lexer(std::move(lexer)) {
  advance(); // Load first token
}

std::string Parser::inferTypeFromExpression(Expr *expr) {
  if (auto literal = dynamic_cast<LiteralExpr*>(expr)) {
    if (std::holds_alternative<int>(literal->value))
      return "số nguyên";
    if (std::holds_alternative<double>(literal->value))
      return "số thực";
    if (std::holds_alternative<std::string>(literal->value))
      return "chuỗi";
    if (std::holds_alternative<bool>(literal->value))
      return "luận lý";
  }
  throw std::runtime_error("Cannot infer type from expression.");
}

std::string Parser::handleType() {
  if (current.value == "số") {
    advance();
    if (check(TokenType::Identifier)) {
      if (current.value == "nguyên") {
        advance();
        return "số nguyên";
      }
      if (current.value == "thực") {
        advance();
        return "số thực";
      }
    } else {
      return "số";
    }
  } else if (current.value == "luận") {
    advance();
    if (check(TokenType::Identifier)) {
      if (current.value == "lý") {
        advance();
        return "luận lý";
      }
    } else {
      return "luận";
    }
  } else {
    std::string temp = current.value;
    advance();
    return temp;
  }
  // Default return to handle all control paths
  throw std::runtime_error("Unexpected type encountered in handleType().");
}

int Parser::getPrecedence(const std::string &op) {
  if (op == "||")
    return 1;
  if (op == "&&")
    return 2;
  if (op == "==" || op == "!=")
    return 3;
  if (op == "<" || op == "<=" || op == ">" || op == ">=")
    return 4;
  if (op == "+" || op == "-")
    return 5;
  if (op == "*" || op == "/" || op == "%")
    return 6;
  return 0;
}