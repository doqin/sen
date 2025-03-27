#pragma once
#include <string>
#include <token.hpp>

class Lexer {
public:
  explicit Lexer(const std::string &source);
  Token nextToken();

private:
  std::string source;
  size_t pos;

  char peek() const;
  char advance();
  static bool isAlpha(const std::string &utf8Char);
  bool isDigit(char c) const;
  int utf8CharSize(unsigned char firstByte);
  std::string extractUtf8Char(const std::string &text, size_t &pos);
};