#pragma once
#include <string>

enum class TokenType {
  Var,
  Identifier,
  Number,
  String,
  Operator,
  OpenParen, CloseParen,
  OpenBrace, CloseBrace,
  Comma, Semicolon,
  EndOfFile,
  Unknown,
  Error
};

struct Token {
  TokenType type;
  std::string value;
};

void printTokenType(TokenType type);