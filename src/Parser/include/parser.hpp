#pragma once
#include <AST.hpp>
#include <lexer.hpp>
#include <memory>
#include <token.hpp>
#include <vector>

class Parser {
private:
  Lexer lexer;
  Token current;

  void advance();
  bool match(TokenType type);
  // Handle primary expression like numbers, strings, identifiers
  std::unique_ptr<Expr> parsePrimary();
  // Handle unary expressions
  std::unique_ptr<Expr> parseUnary();
  // Handle binary expressions
  std::unique_ptr<Expr> parseExpression();
  // Handle function call expressions
  std::unique_ptr<Expr> parseCall(std::unique_ptr<Expr> callee);
  // Handle statements (for now, just expressions)
  std::unique_ptr<Stmt> parseStatement();

public:
  explicit Parser(Lexer lexer);
  // Parses the program from the lexer hehe
  std::vector<std::unique_ptr<Stmt>> parseProgram();
};