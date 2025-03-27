#include "AST.hpp"
#include "token.hpp"
#include <memory>
#include <parser.hpp>
#include <stdexcept>
#include <string>

void Parser::advance() { current = lexer.nextToken(); }

bool Parser::match(TokenType type) {
  if (current.type == type) {
    advance();
    return true;
  }
  return false;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (match(TokenType::Number) || match(TokenType::String)) {
    return std::make_unique<LiteralExpr>(current.value);
  }

  if (match(TokenType::Identifier)) {
    auto identifier = std::make_unique<IdentifierExpr>(current.value);
    return parseCall(std::move(identifier)); // Check if it's a function call
  }

  if (match(TokenType::OpenParen)) {
    auto expr = parseExpression(); // Recursively parse inside parentheses
    if (!match(TokenType::CloseParen)) {
      throw std::runtime_error("Parser::Expected ')' after expression");
    }
    return expr;
  }

  throw std::runtime_error("Parser::Unexpected token: " + current.value);
}

std::unique_ptr<Expr> Parser::parseUnary() {
  if (match(TokenType::Operator) &&
      (current.value == "-" || current.value == "!" || current.value == "~")) {
    std::string op = current.value;
    auto right = parseUnary();
    return std::make_unique<UnaryExpr>(op, std::move(right));
  }
  return parsePrimary();
}

std::unique_ptr<Expr> Parser::parseExpression() {
  auto left = parseUnary();

  while (match(TokenType::Operator)) {
    std::string op = current.value;
    auto right = parseUnary();
    left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
  }

  return left;
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
  auto expr = parseExpression();
  return std::make_unique<ExprStmt>(std::move(expr));
}

Parser::Parser(Lexer lexer) : lexer(std::move(lexer)) {
  advance(); // Load first token
}

std::vector<std::unique_ptr<Stmt>> Parser::parseProgram() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (current.type != TokenType::EndOfFile) {
    statements.push_back(parseStatement());
  }
  return statements;
}