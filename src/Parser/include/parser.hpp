#pragma once
#include <AST.hpp>
#include <lexer.hpp>
#include <memory>
#include <token.hpp>
#include <vector>

struct ParseError : public std::runtime_error {
  int line, column;
  ParseError(int line, int column, const std::string &message)
      : std::runtime_error("Line " + std::to_string(line) + ", Column " +
                           std::to_string(column) + ": " + message),
        line(line), column(column) {}
};
class Parser {
private:
  Lexer lexer;
  Token current;

  void advance();
  bool check(TokenType type) const;
  bool match(TokenType type);

  std::string getLineSnippet(int errorLine);

  // Handle primary expression like numbers, strings, identifiers
  std::unique_ptr<Expr> parsePrimary();
  // Handle unary expressions
  std::unique_ptr<Expr> parseUnary();
  // Handle binary expressions
  std::unique_ptr<Expr> parseBinary(int precedence, std::unique_ptr<Expr> left);
  // Handle expressions
  std::unique_ptr<Expr> parseExpression();
  // Handle function call expressions
  std::unique_ptr<Expr> parseCall(std::unique_ptr<Expr> callee);
  // Handle block statements
  std::unique_ptr<Stmt> parseBlockStatement();
  // Handle if statements
  std::unique_ptr<Stmt> parseIfStatement();
  // Handle while statements
  std::unique_ptr<Stmt> parseWhileStatement();
  // Handle for statements
  std::unique_ptr<Stmt> parseForStatement();
  // Handle for variable declaration statements
  std::unique_ptr<Stmt> parseVarDeclStatement();
  // Handle for function declaration statements
  std::unique_ptr<Stmt> parseFunction();
  // Handle statements
  std::unique_ptr<Stmt> parseStatement();

  // Get precedence hehe
  int getPrecedence(const std::string& op);
public:
  explicit Parser(Lexer lexer);
  // Parses the program from the lexer hehe
  std::vector<std::unique_ptr<Stmt>> parseProgram();
  // Reports the error if catches one
  void reportError(const ParseError& e);
};