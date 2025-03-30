#pragma once
#include <string>

enum class TokenType {
  Var,
  Identifier,
  Number,
  String,
  Operator,
  OpenParen,
  CloseParen,
  OpenBrace,
  CloseBrace,
  Comma,
  Semicolon,
  Colon,
  If,
  Else,
  While,
  For,
  Function,
  EndOfFile,
  Unknown,
  Error
};

struct Token {
  TokenType type;
  std::string value;
  int line, column;

  Token() {}
  Token(TokenType type, std::string value, int line, int column)
      : type(type), value(std::move(value)), line(line), column(column) {}
};

void printTokenType(TokenType type);