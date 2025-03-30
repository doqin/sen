#include "token.hpp"
#include <FileReader.hpp>
#include <exception>
#include <iostream>
#include <lexer.hpp>
#include <parser.hpp>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;
  }

  std::string source;
  try {
    FileReader reader(argv[1]);
    if (std::find(argv, argv + argc, std::string("--show-content")) != argv + argc) {
      std::cout << "File content:\n" << reader.getContent() << std::endl;
    }
    source = reader.getContent();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  if (std::find(argv, argv + argc, std::string("--show-tokens")) != argv + argc) {
    Lexer temp(source);
    Token token;
    while ((token = temp.nextToken()).type != TokenType::EndOfFile) {
      std::cout << "Token: " << token.value << " ";
      printTokenType(token.type);
      std::cout << std::endl;
    }
  }

  Lexer lexer(source);
  Parser parser(lexer);
  try {
    auto program = parser.parseProgram();
    std::cout << "sen::Parsing complete!" << std::endl;
  } catch (ParseError& e) {
    std::cerr << "sen::Parsing error: " << std::endl;
    parser.reportError(e);
  } catch (std::exception& e) {
    std::cerr << "sen::Parsing error: " << e.what() << std::endl;
  }

  return 0;
}