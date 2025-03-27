#pragma once
#include <token.hpp>
#include <lexer.hpp>
#include <memory>
#include <vector>
#include <AST.hpp>

class Parser {
private:
  Lexer lexer;
  Token current;

  void advance();
  bool match(TokenType type);
  // Handle primary expression like numbers, strings, identifiers
  std::unique_ptr<Expr> parsePrimary();
  // Handle binary expressions
  std::unique_ptr<Expr> parseExpression();
  // Handle statements (for now, just expressions)
  std::unique_ptr<Stmt> parseStatement();
public:
  explicit Parser(Lexer lexer);
  // Parses the program from the lexer hehe
  std::vector<std::unique_ptr<Stmt>> parseProgram();
};