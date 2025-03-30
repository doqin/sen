#include "AST.hpp"
#include "token.hpp"
#include <memory>
#include <optional>
#include <parser.hpp>
#include <string>
#include <sstream>
#include <iostream>

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

void Parser::reportError(const ParseError& e) {
  std::cerr << e.what() << std::endl;
  std::string lineContent = getLineSnippet(e.line);
  std::cerr << "  " << lineContent << std::endl;
  std::cerr << "  " << std::string(e.column - 1, ' ') << "^" << std::endl;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (check(TokenType::Number) || check(TokenType::String)) {
    auto expr = std::make_unique<LiteralExpr>(current.value);
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
      throw ParseError(current.line, current.column, "Expected ')' after expression");
    }
    return expr;
  }

  throw ParseError(current.line, current.column, "Unexpected token: " + current.value);
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
      throw ParseError(current.line, current.column, "Expected ')' after function arguments");
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
    throw ParseError(current.column, current.line, "Expected '}' at the end of a block");
  }

  return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
  if (!match(TokenType::OpenParen)) {
    throw ParseError(current.line, current.column, "Expected '(' after 'nếu'");
  }

  auto condition = parseExpression();

  if (!match(TokenType::CloseParen)) {
    throw ParseError(current.line, current.column, "Expected ')' after nếu condition");
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
    throw ParseError(current.line, current.column, "Expected '(' after 'trong khi'");
  }

  auto condition = parseExpression();

  if (!match(TokenType::CloseParen)) {
    throw ParseError(current.line, current.column, "Expected ')' after while condition");
  }

  auto body = parseStatement();

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseForStatement() {
  if (!match(TokenType::OpenParen)) {
    throw ParseError(current.line, current.column, "Expected '(' after 'cho'");
  }

  std::unique_ptr<Stmt> initializer;
  if (!match(TokenType::Semicolon)) {
    initializer = parseStatement();
  }

  auto condition = match(TokenType::Semicolon) ? nullptr : parseExpression();
  if (!match(TokenType::Semicolon)) {
    throw ParseError(current.line, current.column, "Expected ';' after loop condition");
  }

  std::unique_ptr<Expr> increment =
      match(TokenType::CloseParen) ? nullptr : parseExpression();
  if (!match(TokenType::CloseParen)) {
    throw ParseError(current.line, current.column, "Expected ')' after for clause");
  }

  auto body = parseStatement();

  return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                                   std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseVarDeclStatement() {
  if (!check(TokenType::Identifier)) {
    throw ParseError(current.line, current.column, "Expected variable name after 'biến'");
  }
  std::string name = current.value;
  advance();

  std::optional<std::string> typeAnnotation;
  if (match(TokenType::Colon)) { // Check for optional type annotation
    if (!check(TokenType::Identifier)) {
      throw ParseError(current.line, current.column, "Expected type name after ':'");
    }
    typeAnnotation = current.value;
    advance();
  }

  std::unique_ptr<Expr> initializer = nullptr;
  // Check if current token is an operator and specifically "="
  if (check(TokenType::Operator) && current.value == "=") {
    advance(); // Now advance past the "=" operator
    initializer = parseExpression();
  }

  if (!initializer && !typeAnnotation) {
    throw ParseError(current.line, current.column, 
        "Uninitialized variable must have a type annotation.");
  }

  if (!match(TokenType::Semicolon)) {
    throw ParseError(current.line, current.column, "Expected ';' after variable declaration");
  }

  return std::make_unique<VarDeclStmt>(name, std::move(typeAnnotation),
                                       std::move(initializer));
}

std::unique_ptr<Stmt> Parser::parseFunction() {
  if (!check(TokenType::Identifier)) {
    throw ParseError(current.line, current.column, "Expected function name");
  }

  std::string name = current.value;
  advance();

  if (!match(TokenType::OpenParen)) {
    throw ParseError(current.line, current.column, "Expected '(' after function name");
  }

  std::vector<std::pair<std::string, std::string>> parameters;
  if (!check(TokenType::CloseParen)) { // Handle parameters
    do {
      if (!check(TokenType::Identifier)) {
        throw ParseError(current.line, current.column, "Expected parameter name");
      }
      std::string paramName = current.value;
      advance();

      std::string paramType;
      if (!match(TokenType::Colon)) {
        throw ParseError(current.line, current.column, "Expected : after parameter name");
      }

      if (!check(TokenType::Identifier)) {
        throw ParseError(current.line, current.column, "Expected parameter type");
      }
      paramType = current.value; // TODO: Need to check for type
      advance();

      parameters.emplace_back(paramName, paramType);
    } while (match(TokenType::Comma));

    if (!match(TokenType::CloseParen)) {
      throw ParseError(current.line, current.column, "Expected ')' after parameters");
    }
  }

  std::string returnType = "rỗng"; // Rename it to something else
  if (match(TokenType::Colon)) {
    if (!check(TokenType::Identifier)) {
      throw ParseError(current.line, current.column, "Expected return type after ':'");
    }
    returnType = current.value; // TODO: Check for Token type
    advance();
  }

  if (!match(TokenType::OpenBrace)) {
    throw ParseError(current.line, current.column, "Expected '{' before function body");
  }

  std::unique_ptr<Stmt> bodyStmt = parseBlockStatement();
  std::unique_ptr<BlockStmt> body(
      dynamic_cast<BlockStmt *>(bodyStmt.release()));

  if (!body) {
    throw ParseError(current.line, current.column, "Expected a block statement for function body");
  }
  return std::make_unique<FunctionStmt>(name, std::move(parameters), returnType,
                                        std::move(body));
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

  auto expr = parseExpression();
  if (!match(TokenType::Semicolon)) {
    throw ParseError(current.line, current.column, "Expected ';' after expression");
  }
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::vector<std::unique_ptr<Stmt>> Parser::parseProgram() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (current.type != TokenType::EndOfFile) {
    statements.push_back(parseStatement());
  }
  return statements;
}

Parser::Parser(Lexer lexer) : lexer(std::move(lexer)) {
  advance(); // Load first token
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