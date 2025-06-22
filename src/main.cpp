#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./quanta <filename>\n";
    return 1;
  }

  std::ifstream file(argv[1]);
  if (!file) {
    std::cerr << "Failed to open file: " << argv[1] << "\n";
    return 1;
  }

  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

  Lexer lexer(source);
  auto tokens = lexer.tokenize();

  Parser parser(tokens);
  auto program = parser.parse();

  return 0;
}
