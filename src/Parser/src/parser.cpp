#include "AST.hpp"
#include "token.hpp"
#include <memory>
#include <parser.hpp>
#include <stdexcept>

void Parser::advance() {
  current = lexer.nextToken();
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
        return std::make_unique<LiteralExpr>(current.value);
    }

    if (match(TokenType::Identifier)) {
        return std::make_unique<LiteralExpr>(current.value);
    }

    throw std::runtime_error("Parser::Unexpected token: " + current.value);
}

std::unique_ptr<Expr> Parser::parseExpression() {
    std::unique_ptr<Expr> left = parsePrimary();

    while (match(TokenType::Operator)) {
        std::string op = current.value;
        std::unique_ptr<Expr> right = parsePrimary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
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