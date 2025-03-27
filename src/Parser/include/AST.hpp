#pragma once
#include <memory>
#include <string>
#include <vector>

class Expr {
public:
  virtual ~Expr() = default;
};

// Expression for literals (numbers, strings, etc.)
class LiteralExpr : public Expr {
public:
  std::string value;
  explicit LiteralExpr(const std::string &value) : value(value) {}
};

// Expression for identifiers (variables, function names)
class IdentifierExpr : public Expr {
public:
  std::string name;
  explicit IdentifierExpr(const std::string &name) : name(name) {}
};

// Unary expression like -5, !true, and ~x
class UnaryExpr : public Expr {
public:
  std::string op;
  std::unique_ptr<Expr> right;

  UnaryExpr(std::string op, std::unique_ptr<Expr> right)
      : op(std::move(op)), right(std::move(right)) {}
};

// Binary expression like 1 + 2
class BinaryExpr : public Expr {
public:
  std::unique_ptr<Expr> left;
  std::string op;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::unique_ptr<Expr> left, std::string op,
             std::unique_ptr<Expr> right)
      : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

class CallExpr : public Expr {
public:
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> arguments;

  CallExpr(std::unique_ptr<Expr> callee,
           std::vector<std::unique_ptr<Expr>> args)
      : callee(std::move(callee)), arguments(std::move(args)) {}
};

// Statement base class
class Stmt {
public:
  virtual ~Stmt() = default;
};

// Expression statement (for standalone expression)
class ExprStmt : public Stmt {
public:
  std::unique_ptr<Expr> expression;
  explicit ExprStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
};