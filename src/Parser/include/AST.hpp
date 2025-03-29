#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Base class for all expressions
struct Expr {
  virtual ~Expr() = default;
};

// Declaration
struct BlockStmt;
struct VarDeclStmt;
struct ExprStmt;
struct IfStmt;

/*
// Base class for StmtVisitor
struct StmtVisitor {
  virtual void visitBlockStmt(const BlockStmt &stmt) = 0;
  virtual void visitVarDeclStmt(const VarDeclStmt &stmt) = 0;
  virtual void visitExprStmt(const ExprStmt &stmt) = 0;
  virtual void visitIfStmt(const IfStmt &stmt) = 0;

  virtual ~StmtVisitor() = default;
};
*/

// Base class for all statements
struct Stmt {
  /*
  virtual void accept(StmtVisitor &visitor) const = 0;
  */
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

// If statement (e.g., `nếu (a < b) {} khác`)
struct IfStmt : public Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> thenBranch;
  std::unique_ptr<Stmt> elseBranch;

  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch,
         std::unique_ptr<Stmt> elseBranch)
      : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
        elseBranch(std::move(elseBranch)) {}
};

// While statement (e.g., `trong khi (a < b) {})
struct WhileStmt : public Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;

  WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
      : condition(std::move(condition)), body(std::move(body)) {}
};

// For statement (e.g. `cho (expr; condition; incre) {}`)
struct ForStmt : public Stmt {
  std::unique_ptr<Stmt> initializer;
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> increment;
  std::unique_ptr<Stmt> body;

  ForStmt(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition,
          std::unique_ptr<Expr> increment, std::unique_ptr<Stmt> body)
      : initializer(std::move(initializer)), condition(std::move(condition)),
        increment(std::move(increment)), body(std::move(body)) {}
};

// Block statement (e.g., `{ x = 10; y = 20; }`)
struct BlockStmt : public Stmt {
  std::vector<std::unique_ptr<Stmt>> statements;

  BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
      : statements(std::move(stmts)) {}

  /*
  void accept(StmtVisitor &visitor) const override {
    visitor.visitBlockStmt(*this);
  }
  */
};

struct FunctionStmt : Stmt {
  std::string name;
  std::vector<std::string> parameters;
  std::unique_ptr<BlockStmt> body;

  FunctionStmt(std::string name, std::vector<std::string> params,
               std::unique_ptr<BlockStmt> body)
      : name(std::move(name)), parameters(std::move(params)),
        body(std::move(body)) {}
};

// Print function (for debugging AST)
void printAST(const std::unique_ptr<Stmt> &stmt, int indent = 0);