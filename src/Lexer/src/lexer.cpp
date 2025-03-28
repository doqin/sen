#include "token.hpp"
#include <cctype>
#include <cstddef>
#include <lexer.hpp>

Lexer::Lexer(const std::string &source) : source(source), pos(0) {}

char Lexer::peek() const {
  return (pos < source.length()) ? source[pos] : '\0';
}

char Lexer::advance() {
  if (pos < source.length())
    return source[pos++];
  return '\0';
}

bool Lexer::isAlpha(const std::string &utf8Char) {
  if (utf8Char.empty())
    return false;

  unsigned char firstByte = static_cast<unsigned char>(utf8Char[0]);

  if ((firstByte >= 'a' && firstByte <= 'z') ||
      (firstByte >= 'A' && firstByte <= 'Z') || firstByte == '_') {
    return true;
  }

  return utf8Char.size() >= 2;
}

bool Lexer::isDigit(char c) const { return std::isdigit(c); }

int Lexer::utf8CharSize(unsigned char firstByte) {
  if ((firstByte & 0x80) == 0)
    return 1; // 1-byte (ASCII)
  if ((firstByte & 0xE0) == 0xC0)
    return 2; // 2-byte character
  if ((firstByte & 0xF0) == 0xE0)
    return 3; // 3-byte character
  if ((firstByte & 0xF8) == 0xF0)
    return 4; // 4-byte character
  return 1;   // Fallback (treat as ASCII)
}

std::string Lexer::extractUtf8Char(const std::string &text, size_t &pos) {
  if (pos >= text.length())
    return ""; // Prevent out-of-bounds access

  int charSize = utf8CharSize(static_cast<unsigned char>(text[pos]));

  // Prevent reading beyond the string length
  if (pos + charSize > text.length())
    charSize = text.length() - pos;

  std::string utf8Char = text.substr(pos, charSize);
  pos += charSize; // Move position forward
  return utf8Char;
}

Token Lexer::nextToken() {
  while (std::isspace(peek()))
    advance();

  if (pos >= source.length())
    return {TokenType::EndOfFile, ""};

  size_t startPos = pos;
  std::string firstChar = extractUtf8Char(
      source, pos); // Note: pos is passed as reference and updated

  // Check if the first character is a quote
  if (firstChar[0] == '"') {
    std::string str = "";
    while (peek() != '"' && peek() != '\0') {
      str += advance();
    }

    if (peek() == '"') {
      advance(); // Consume the closing quote
      return {TokenType::String, "\"" + str + "\""};
    } else {
      return {TokenType::Error, "Unterminated string"};
    }
  }

  // Handles alphabetic characters
  if (isAlpha(firstChar)) {
    std::string identifier = firstChar;

    while (pos < source.length()) {
      size_t tempPos = pos;
      std::string nextChar = extractUtf8Char(source, tempPos);
      if (isAlpha(nextChar) || (nextChar.size() == 1 && isDigit(nextChar[0]))) {
        identifier += nextChar;
        pos = tempPos; // Update pos only if we used the character
      } else {
        break;
      }
    }

    // Check for multi-word keywords like "trong khi"
    if (identifier == "trong") {
      size_t tempPos = pos;

      // Skip whitespace after "trong"
      while (std::isspace(peek())) advance();

      std::string nextWord;
      if (isAlpha(std::string(1, peek()))) {
        nextWord = extractUtf8Char(source, pos);

        // Extract remaining characters for the second word
        while (pos < source.length()) {
          size_t temp2Pos = pos;
          std::string nextChar = extractUtf8Char(source, temp2Pos);
          if (isAlpha(nextChar) || (nextChar.size() == 1 && isDigit(nextChar[0]))) {
            nextWord += nextChar;
            pos = temp2Pos; // Update pos only if we used the character
          } else {
            break;
          }
        }
      }

      if (nextWord == "khi") {
        return { TokenType::While, "trong khi"};
      } 
      
      pos = tempPos;
    }

    // Check for multi-word keywords like "không thì"
    if (identifier == "không") {
      size_t tempPos = pos;

      // Skip whitespace after "trong"
      while (std::isspace(peek())) advance();

      std::string nextWord;
      if (isAlpha(std::string(1, peek()))) {
        nextWord = extractUtf8Char(source, pos);

        // Extract remaining characters for the second word
        while (pos < source.length()) {
          size_t temp2Pos = pos;
          std::string nextChar = extractUtf8Char(source, temp2Pos);
          if (isAlpha(nextChar) || (nextChar.size() == 1 && isDigit(nextChar[0]))) {
            nextWord += nextChar;
            pos = temp2Pos; // Update pos only if we used the character
          } else {
            break;
          }
        }
      }

      if (nextWord == "thì") {
        return { TokenType::Else, "không thì"};
      } 
      
      pos = tempPos;
    }

    // Recognize other keywords
    if (identifier == "nếu")  return { TokenType::If,       identifier };
    if (identifier == "cho")  return { TokenType::For,      identifier };
    if (identifier == "biến") return { TokenType::Var,      identifier };
    if (identifier == "hàm")  return { TokenType::Function, identifier };
    return {TokenType::Identifier, identifier };
  }

  if (std::isdigit(firstChar[0])) {
    std::string number = firstChar;
    while (std::isdigit(peek()))
      number += advance();
    return {TokenType::Number, number};
  }

  std::string twoCharOps[] = {"==", "!=", ">=", "<=", "&&", "||"};
  for (const auto &op : twoCharOps) {
    if (firstChar[0] == op[0] && pos < source.length() &&
        source[pos] == op[1]) {
      advance(); // Consume second character of the operator
      return {TokenType::Operator, op};
    }
  }

  std::string operators = "+-*/%<>=!&|";
  if (operators.find(firstChar[0]) != std::string::npos) {
    return {TokenType::Operator, firstChar};
  }

  switch (firstChar[0]) {
  case '(':
    return {TokenType::OpenParen, "("};
  case ')':
    return {TokenType::CloseParen, ")"};
  case '{':
    return {TokenType::OpenBrace, "{"};
  case '}':
    return {TokenType::CloseBrace, "}"};
  case '"':
    return {TokenType::String, "\""}; // Not sure if this is necessary
  case ',':
    return { TokenType::Comma, ","};
  case ';':
    return { TokenType::Semicolon, ";"};
  case ':':
    return { TokenType::Colon, ":"};
  case '\0':
    return {TokenType::EndOfFile, ""};
  default:
    return {TokenType::Unknown, firstChar};
  }
}