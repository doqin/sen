#include "AST.hpp"
#include "token.hpp"
#include <memory>
#include <parser.hpp>
#include <stdexcept>
#include <optional>
#include <string>

void Parser::advance() {
  current = lexer.nextToken();
}

bool Parser::check(TokenType type) const {
  return current.type == type;
}

bool Parser::match(TokenType type) {
  if (current.type == type) {
    advance();
    return true;
  }
  return false;
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
      throw std::runtime_error("Expected ')' after expression");
    }
    return expr;
  }

  throw std::runtime_error("Unexpected token: " + current.value);
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
      throw std::runtime_error("Expected ')' after function arguments");
    }
  }

  return std::make_unique<CallExpr>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Stmt> Parser::parseBlockStatement() {
  std::vector<std::unique_ptr<Stmt>> statements;

  while (!check(TokenType::CloseBrace) &&
         !check(TokenType::EndOfFile)) {
    statements.push_back(parseStatement());
  }

  if (!match(TokenType::CloseBrace)) {
    throw std::runtime_error("Expected '}' at the end of a block");
  }

  return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
  if (!match(TokenType::OpenParen)) {
    throw std::runtime_error("Expected '(' after 'nếu'");
  }

  auto condition = parseExpression();

  if (!match(TokenType::CloseParen)) {
    throw std::runtime_error("Expected ')' after nếu condition");
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
    throw std::runtime_error("Expected '(' after 'trong khi'");
  }

  auto condition = parseExpression();

  if (!match(TokenType::CloseParen)) {
    throw std::runtime_error("Expected ')' after while condition");
  }

  auto body = parseStatement();

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseForStatement() {
  if (!match(TokenType::OpenParen)) {
    throw std::runtime_error("Expected '(' after 'cho'");
  }

  std::unique_ptr<Stmt> initializer;
  if (!match(TokenType::Semicolon)) {
    initializer = parseStatement();
  }

  auto condition = match(TokenType::Semicolon) ? nullptr : parseExpression();
  if (!match(TokenType::Semicolon)) {
    throw std::runtime_error("Expected ';' after loop condition");
  }

  std::unique_ptr<Expr> increment =
      match(TokenType::CloseParen) ? nullptr : parseExpression();
  if (!match(TokenType::CloseParen)) {
    throw std::runtime_error("Expected ')' after for clause");
  }

  auto body = parseStatement();

  return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                                   std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseVarDeclStatement() {
  if (!check(TokenType::Identifier)) {
    throw std::runtime_error("Expected variable name after 'biến'");
  }
  std::string name = current.value;
  advance();

  std::optional<std::string> typeAnnotation;
  if (match(TokenType::Colon)) { // Check for optional type annotation
    if (!check(TokenType::Identifier)) {
      throw std::runtime_error("Expected type name after ':'");
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
    throw std::runtime_error("Uninitialized variable must have a type annotation.");
  }

  if (!match(TokenType::Semicolon)) {
    throw std::runtime_error("Expected ';' after variable declaration");
  }

  return std::make_unique<VarDeclStmt>(name, std::move(typeAnnotation), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::parseFunction() {
  if (!check(TokenType::Identifier)) {
    throw std::runtime_error("Expected function name");
  }

  std::string name = current.value;
  advance();

  if (!match(TokenType::OpenParen)) {
    throw std::runtime_error("Expected '(' after function name");
  }

  std::vector<std::pair<std::string, std::string>> parameters;
  if (!check(TokenType::CloseParen)) { // Handle parameters
    do {
      if (!check(TokenType::Identifier)) {
        throw std::runtime_error("Expected parameter name");
      }
      std::string paramName = current.value;
      advance();

      std::string paramType;
      if (!match(TokenType::Colon)) {
        throw std::runtime_error("Expected : after parameter name");
      }

      if (!check(TokenType::Identifier)) {
        throw std::runtime_error("Expected parameter type");
      }
      paramType = current.value; // TODO: Need to check for type
      advance();

      parameters.emplace_back(paramName, paramType);
    } while (match(TokenType::Comma));

    if (!match(TokenType::CloseParen)) {
      throw std::runtime_error("Expected ')' after parameters");
    }
  }

  std::string returnType = "rỗng"; // Rename it to something else
  if (match(TokenType::Colon)) {
    if (!check(TokenType::Identifier)) {
      throw std::runtime_error("Expected return type after ':'");
    }
    returnType = current.value; // TODO: Check for Token type
    advance();
  }

  if (!match(TokenType::OpenBrace)) {
    throw std::runtime_error("Expected '{' before function body");
  }

  std::unique_ptr<Stmt> bodyStmt = parseBlockStatement();
  std::unique_ptr<BlockStmt> body(dynamic_cast<BlockStmt*>(bodyStmt.release()));

  if (!body) {
    throw std::runtime_error("Expected a block statement for function body");
  }
  return std::make_unique<FunctionStmt>(name, std::move(parameters), returnType, std::move(body));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
  if (match(TokenType::If)) return parseIfStatement();
  if (match(TokenType::While)) return parseWhileStatement();
  if (match(TokenType::For)) return parseForStatement();
  if (match(TokenType::OpenBrace)) return parseBlockStatement();
  if (match(TokenType::Var)) return parseVarDeclStatement();
  if (match(TokenType::Function)) return parseFunction();

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