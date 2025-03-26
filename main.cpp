#include <exception>
#include <iostream>
#include <lexer.hpp>
#include <FileReader.hpp>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string source;
    try {
        FileReader reader(argv[1]);
        std::cout << "File content:\n" << reader.getContent() << std::endl;
        source = reader.getContent();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }


    Lexer lexer(source);
    Token token;
    while ((token = lexer.nextToken()).type != TokenType::EndOfFile) {
        std::cout << "Token: " << token.value << std::endl;
    }

    return 0;
}