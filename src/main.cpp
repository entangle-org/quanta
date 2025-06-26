#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "codegen/cppgen.hpp"
#include "codegen/oqasmgen.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: quanta <input.qt>\n";
    return 1;
  }

  std::ifstream in(argv[1]);
  if (!in.is_open()) {
    std::cerr << "Error: could not open input file.\n";
    return 1;
  }

  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string source = buffer.str();

  Lexer lexer(source);
  auto tokens = lexer.tokenize();
  Parser parser(tokens);
  auto program = parser.parse();

  std::cout << "==================== C++ OUTPUT ====================\n";
  CppGenerator cpp;
  program->accept(cpp);
  std::cout << cpp.str() << "\n";

  std::cout << "================= OPENQASM OUTPUT ==================\n";
  QasmGenerator qasm;
  program->accept(qasm);
  std::cout << qasm.str() << "\n";

  return 0;
}
