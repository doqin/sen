#include <memory>
#include <parser.hpp>
#include <stdexcept>
#include <string>

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

std::unique_ptr<Stmt> Parser::parseBlockStatement() {
  std::vector<std::unique_ptr<Stmt>> statements;

  while (current.type != TokenType::CloseBrace &&
         current.type != TokenType::EndOfFile) {
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
  if (!match(TokenType::Identifier)) {
    throw std::runtime_error("Expected variable name after 'biến'");
  }
  std::string name = current.value;

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

std::unique_ptr<Stmt> Parser::parseStatement() {
  if (match(TokenType::If)) return parseIfStatement();
  if (match(TokenType::While)) return parseWhileStatement();
  if (match(TokenType::For)) return parseForStatement();
  if (match(TokenType::OpenBrace)) return parseBlockStatement();
  if (match(TokenType::Var)) return parseVarDeclStatement();

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