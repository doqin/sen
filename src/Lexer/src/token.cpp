#include <token.hpp>
#include <iostream>

void printTokenType(TokenType type) {
    switch(type) {
      case TokenType::Var:        std::cout << "(Var)";         break;
      case TokenType::Identifier: std::cout << "(Identifier)";  break;
      case TokenType::Number:     std::cout << "(Number)";      break;
      case TokenType::String:     std::cout << "(String)";      break;
      case TokenType::Operator:   std::cout << "(Operator)";    break;
      case TokenType::OpenParen:  std::cout << "(OpenParen)";   break;
      case TokenType::CloseParen: std::cout << "(CloseParen)";  break;
      case TokenType::OpenBrace:  std::cout << "(OpenBrace)";   break;
      case TokenType::CloseBrace: std::cout << "(CloseBrace)";  break;
      case TokenType::Comma:      std::cout << "(Comma)";       break;
      case TokenType::Semicolon:  std::cout << "(Semicolon)";   break;
      case TokenType::Colon:      std::cout << "(Colon)";       break;
      case TokenType::If:         std::cout << "(If)";          break;
      case TokenType::Else:       std::cout << "(Else)";        break;
      case TokenType::While:      std::cout << "(While)";       break;
      case TokenType::For:        std::cout << "(For)";         break;
      case TokenType::Function:   std::cout << "(Function)";    break;
      case TokenType::EndOfFile:  std::cout << "(EndOfFile)";   break;
      case TokenType::Unknown:    std::cout << "(Unknown)";     break;
      case TokenType::Error:      std::cout << "(Error)";       break;
      default: break;
    }
  }