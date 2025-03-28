#include "token.hpp"
#include <iostream>
#include <memory>
#include <parser.hpp>
#include <stdexcept>
#include <string>

void Parser::advance() { 
  current = lexer.nextToken(); 
  std::cout << "Parser::Current Token: " << current.value << " ";
  printTokenType(current.type);
  std::cout << std::endl;
}

bool Parser::match(TokenType type) {
  if (current.type == type) {
    advance();
    return true;
  }
  return false;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (match(TokenType::Number) || match(TokenType::String)) {
    auto expr = std::make_unique<LiteralExpr>(current.value);
    // Remove the advance() call here
    return expr;
  }

  if (match(TokenType::Identifier)) {
    auto identifier = std::make_unique<VarExpr>(current.value);
    // Remove the advance() call here
    return parseCall(std::move(identifier)); // Check if it's a function call
  }

  if (match(TokenType::OpenParen)) {
    auto expr = parseExpression(); // Recursively parse inside parentheses
    if (!match(TokenType::CloseParen)) {
      throw std::runtime_error("Expected ')' after expression");
    }
    return expr;
  }

  throw std::runtime_error("Unexpected token: " + current.value);
}

std::unique_ptr<Expr> Parser::parseUnary() {
  if (match(TokenType::Operator) &&
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
  while (current.type == TokenType::Operator) {
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
  if (!match(TokenType::CloseParen)) { // Handle empty argument list
    do {
      arguments.push_back(parseExpression());
    } while (match(TokenType::Comma));

    if (!match(TokenType::CloseParen)) {
      throw std::runtime_error("Expected ')' after function arguments");
    }
  }

  return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
  if (match(TokenType::Var)) {
    if (!match(TokenType::Identifier)) {
      throw std::runtime_error("Expected variable name after 'biến'");
    }
    std::string name = current.value;
    // Remove this line - match() already advanced the token:
    // advance(); 

    std::unique_ptr<Expr> initializer = nullptr;
    // Check if current token is an operator and specifically "="
    if (current.type == TokenType::Operator && current.value == "=") {
      advance(); // Now advance past the "=" operator
      initializer = parseExpression();
    }

    if (!match(TokenType::Semicolon)) {
      throw std::runtime_error("Expected ';' after variable declaration");
    }

    return std::make_unique<VarDeclStmt>(name, std::move(initializer));
  }

  auto expr = parseExpression();
  if (!match(TokenType::Semicolon)) {
    throw std::runtime_error("Expected ';' after expression");
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