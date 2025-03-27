#pragma once
#include <string>

enum class TokenType {
  Identifier,
  Number,
  String,
  Operator,
  OpenParen,
  CloseParen,
  OpenBrace,
  CloseBrace,
  Comma,
  EndOfFile,
  Unknown,
  Error
};

struct Token {
  TokenType type;
  std::string value;
};