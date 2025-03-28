#pragma once
#include <memory>
#include <string>
#include <vector>

// Base class for all expressions
struct Expr {
  virtual ~Expr() = default;
};

// Base class for all statements
struct Stmt {
  virtual ~Stmt() = default;
};

// ──────────────────── EXPRESSIONS ────────────────────

// Expression for literals (numbers, strings, etc.)
struct LiteralExpr : public Expr {
  std::string value;
  explicit LiteralExpr(const std::string &value) : value(value) {}
};

// Expression for identifiers (variables, function names)
struct VarExpr : public Expr {
  std::string name;
  explicit VarExpr(const std::string &name) : name(name) {}
};

// Unary expression like -5, !true, and ~x
struct UnaryExpr : public Expr {
  std::string op;
  std::unique_ptr<Expr> right;

  UnaryExpr(std::string op, std::unique_ptr<Expr> right)
      : op(std::move(op)), right(std::move(right)) {}
};

// Binary expression like 1 + 2
struct BinaryExpr : public Expr {
  std::unique_ptr<Expr> left;
  std::string op;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::unique_ptr<Expr> left, std::string op,
             std::unique_ptr<Expr> right)
      : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

struct CallExpr : public Expr {
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> arguments;

  CallExpr(std::unique_ptr<Expr> callee,
           std::vector<std::unique_ptr<Expr>> args)
      : callee(std::move(callee)), arguments(std::move(args)) {}
};

// ──────────────────── STATEMENTS ────────────────────

// Expression statement (e.g., `print(42);`)
struct ExprStmt : public Stmt {
  std::unique_ptr<Expr> expression;
  explicit ExprStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
};

// Variable declaration statement (e.g., `x = 10;`)
struct VarDeclStmt : public Stmt {
  std::string name;
  std::unique_ptr<Expr> initializer;

  VarDeclStmt(std::string name, std::unique_ptr<Expr> initializer)
      : name(std::move(name)), initializer(std::move(initializer)) {}
};

// Block statement (e.g., `{ x = 10; y = 20; }`)
struct BlockStmt : public Stmt {
  std::vector<std::unique_ptr<Stmt>> statements;
};

// Print function (for debugging AST)
void printAST(const std::unique_ptr<Stmt>& stmt, int indent = 0);